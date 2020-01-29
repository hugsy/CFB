/*++

Module Name:
   main.c

   
Abstract:

This is the main file for the broker.

The broker is a privileged process (must have at least SeDebug and SeLoadDriver) and is responsible 
for communicating with the IrpDumper driver by:
- create the service and load the driver
- instructing the driver to add/remove hooks to specific driver(s)
- fetching IRP data from the hooked drivers
- upon cleanup the resources, unload the driver and delete the service.


--*/


#include "main.h"


Session* Sess;

static HANDLE g_hTerminationEvent;

/*++

Routine Description:

Attempts to acquire a privilege by its name.


Arguments:

	lpszPrivilegeName - the name (as a wide string) of the privilege


Return Value:

	Returns TRUE if the privilege was successfully acquired

--*/
static BOOL AssignPrivilegeToSelf(_In_ const wchar_t* lpszPrivilegeName)
{
	HANDLE hToken = INVALID_HANDLE_VALUE;
	BOOL bRes = FALSE;
	

	bRes = ::OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
	if (bRes)
	{
		LUID Luid = { 0, };

		bRes = ::LookupPrivilegeValue(NULL, lpszPrivilegeName, &Luid);
		if (bRes)
		{
			size_t nBufferSize = sizeof(TOKEN_PRIVILEGES) + 1*sizeof(LUID_AND_ATTRIBUTES);
			PTOKEN_PRIVILEGES NewState = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR, nBufferSize);
			if (NewState)
			{
				NewState->PrivilegeCount = 1;
				NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
				NewState->Privileges[0].Luid = Luid;

				bRes = ::AdjustTokenPrivileges(
					hToken,
					FALSE,
					NewState,
					0,
					(PTOKEN_PRIVILEGES)NULL,
					(PDWORD)NULL
				) != 0;

				if (bRes)
					bRes = GetLastError() != ERROR_NOT_ALL_ASSIGNED;

				LocalFree(NewState);
			}
		}

		CloseHandle(hToken);
	}

	return bRes;
}


/*++

Routine Description:

Simple helper function to check a privilege by name on the current process.


Arguments:

	lpszPrivilegeName - the name (as a wide string) of the privilege

	lpHasPriv - a pointer to a boolean indicating whether the current process own that 
	privilege


Return Value:

	Returns TRUE if the current has the privilege 

--*/
static BOOL HasPrivilege(_In_ const wchar_t* lpszPrivilegeName, _Out_ PBOOL lpHasPriv)
{
	LUID Luid = { 0, };
	BOOL bRes = FALSE, bHasPriv = FALSE;
	HANDLE hToken = INVALID_HANDLE_VALUE;

	do
	{
		xlog(LOG_DEBUG, L"Checking for '%s'...\n", lpszPrivilegeName);

		bRes = LookupPrivilegeValue(NULL, lpszPrivilegeName, &Luid);
		if (!bRes)
		{
			PrintErrorWithFunctionName(L"LookupPrivilegeValue");
			break;
		}

		LUID_AND_ATTRIBUTES PrivAttr = { 0 };
		PrivAttr.Luid = Luid;
		PrivAttr.Attributes = SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT;

		PRIVILEGE_SET PrivSet = { 0, };
		PrivSet.PrivilegeCount = 1;
		PrivSet.Privilege[0] = PrivAttr;

		bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
		if (!bRes)
		{
			PrintError(L"OpenProcessToken");
			break;
		}

		bRes = PrivilegeCheck(hToken, &PrivSet, &bHasPriv);
		if (!bRes)
		{
			PrintError(L"PrivilegeCheck");
			break;
		}

		*lpHasPriv = bHasPriv;
		bRes = TRUE;
	} 
	while (0);

	if (hToken != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hToken);
	}

	return bRes;
}



/*++

Routine Description:

Ctrl-C handler: when receiving either CTRL_C_EVENT or CTRL_CLOSE_EVENT, set the termination event.


Arguments:

	fdwCtrlType - the event type received by the window


Return Value:

	Returns TRUE if the event was handled. FALSE otherwise.

--*/
static BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
	case CTRL_C_EVENT:
		xlog(LOG_INFO, L"Received Ctrl-C event, stopping...\n");
		Sess->Stop();
		return TRUE;

	default:
		return FALSE;
	}
}


/*++

Routine Description:

Run forever loop, can be run from either the standalone mode or own process service.


Arguments:

	lpParameter - unused parameter, here only for WINAPI compatibility.


Return Value:

	Return 0 on success, or the last error code.

--*/
DWORD RunForever(LPVOID lpParameter)
{
	UNREFERENCED_PARAMETER(lpParameter);

	DWORD retcode = ERROR_SUCCESS;
	HANDLE ThreadHandles[3] = { 0 };
	DWORD dwWaitResult = 0;


	//
	// Initialize the broker <-> driver thread
	//

	if (!StartBackendManagerThread(Sess))
	{
		xlog(LOG_CRITICAL, L"StartBackendManagerThread() failed\n");
		return retcode;
	}


	//
	// Initialize the gui <-> broker thread
	//

	if (!FrontendThreadRoutine(Sess))
	{
		xlog(LOG_CRITICAL, L"FrontendThreadRoutine() failed\n");
		return retcode;
	}


	xlog(
		LOG_SUCCESS, 
		L"ThreadIds[]=[Frontend=%d,Backend=%d,IrpFetcher=%d]\n", 
		::GetThreadId(Sess->m_hFrontendThread), 
		::GetThreadId(Sess->m_hBackendThread), 
		::GetThreadId(Sess->m_hIrpFetcherThread)
	);


	//
	// Start everything
	//
	Sess->Start();
	ResumeThread(Sess->m_hFrontendThread);
	ResumeThread(Sess->m_hIrpFetcherThread);
	ResumeThread(Sess->m_hBackendThread);


	//
	// Wait for those 2 threads to finish
	//
	ThreadHandles[0] = Sess->m_hFrontendThread;
	ThreadHandles[1] = Sess->m_hIrpFetcherThread;
	ThreadHandles[2] = Sess->m_hBackendThread;

	dwWaitResult = ::WaitForMultipleObjects(_countof(ThreadHandles), ThreadHandles, TRUE, INFINITE);

	switch (dwWaitResult)
	{
	case WAIT_OBJECT_0:
		xlog(LOG_SUCCESS, L"All threads ended, cleaning up...\n");
		retcode = ERROR_SUCCESS;
		break;

	default:
		PrintErrorWithFunctionName(L"WaitForMultipleObjects");
		retcode = ::GetLastError();
		break;
	}

	return retcode;
}


/*++

Routine Description:

The entrypoint for the broker.


Arguments:

	argc - 

	argv - 


Return Value:

	Returns EXIT_SUCCESS on success, EXIT_FAILURE

--*/
int wmain(int argc, wchar_t** argv)
{
	int retcode = EXIT_SUCCESS;
	HANDLE hDriver = INVALID_HANDLE_VALUE;
	HANDLE hGui = INVALID_HANDLE_VALUE;
	
	
	BOOL bHasDebugPriv = FALSE;
	BOOL bHasLoadDriverPriv = FALSE;
	
	Sess = nullptr;


	HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	DWORD dwConsoleMode;

	::GetConsoleMode(hStdErr, &dwConsoleMode);
	dwConsoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	::SetConsoleMode(hStdErr, dwConsoleMode);


	xlog(LOG_INFO, L"Starting %s (part of %s (v%.02f) - by <%s>)\n", argv[0], CFB_PROGRAM_NAME_SHORT, CFB_VERSION, CFB_AUTHOR);

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"DEBUG mode on\n");
#endif


	//
	// Check the privileges first, if we don't have them there is no point going further
	//
	const wchar_t* lpszPrivilegeNames[2] = { L"SeDebugPrivilege", L"SeLoadDriverPrivilege" };

	for (int i = 0; i < _countof(lpszPrivilegeNames); i++)
	{
		BOOL fHasPriv = FALSE;
		if (!AssignPrivilegeToSelf(lpszPrivilegeNames[i]))
		{
			xlog(LOG_CRITICAL, L"%s requires '%s', cannot proceed...\n", argv[0], lpszPrivilegeNames[i]);
			return EXIT_FAILURE;
		}
	}

	dbg(L"Privilege check succeeded...\n");


	//
	// The session is ready to be initialized
	//

	dbg(L"Initializing the session...\n");
	
	try
	{
		Sess = new Session();
	}
	catch (std::runtime_error& e)
	{
		xlog(LOG_CRITICAL, L"Failed to initialize the session, reason: %S\n", e.what());
		return EXIT_FAILURE;
	}

	do
	{

		//
		// Should we run as a background service
		//
		if (argc > 1 && !wcscmp(argv[1], L"--service"))
		{
			Sess->ServiceManager.bRunInBackground = TRUE;
			if (!Sess->ServiceManager.RegisterService())
			{
				xlog(LOG_CRITICAL, L"Failed to register service...\n");
				retcode = EXIT_FAILURE;
				break;
			}
		}


		if (Sess->ServiceManager.bRunInBackground == FALSE)
		{
			if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
			{
				xlog(LOG_CRITICAL, L"Could not setup SetConsoleCtrlHandler()...\n");
				retcode = EXIT_FAILURE;
				break;
			}
		}


		if (RunForever(NULL) != ERROR_SUCCESS)
		{
			xlog(LOG_ERROR, L"RunForever() returned with error\n");
			break;
		}

		dbg(L"RunForever() finished successfully\n");
	} 
	while (0);


	//
	// Deleting the session will automatically destroy the FrontEnd/BackEnd server, along with 
	// the service manager.
	//
	try
	{
		if (Sess != nullptr)
		{
			delete Sess;
		}
	}
	catch (std::runtime_error & e)
	{
		xlog(LOG_CRITICAL, L"An error occured while deleting the session: %S\n", e.what());
		retcode = EXIT_FAILURE;
	}


#ifdef _DEBUG
	if(retcode != EXIT_SUCCESS)
		xlog(LOG_CRITICAL, L"%s exited with error(s)\n", argv[0]);
#endif // _DEBUG


	return retcode;
}