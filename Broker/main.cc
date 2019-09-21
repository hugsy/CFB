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
	HANDLE ThreadHandles[2] = { 0 };
	DWORD dwWaitResult = 0;
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
	// Check the privileges
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

	xlog(LOG_SUCCESS, L"Privilege check succeeded...\n");


	xlog(LOG_INFO, L"Initializing the session...\n");
	
	try
	{
		Sess = new Session();
	}
	catch (std::runtime_error& e)
	{
		xlog(LOG_CRITICAL, L"Failed to initialize the session, reason: %S\n", e.what());
		return EXIT_FAILURE;
	}



	//
	// Install the Ctrl-C handler to clean up properly
	//
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	{
		xlog(LOG_CRITICAL, L"Could not setup SetConsoleCtrlHandler()...\n");
		delete Sess;
		return EXIT_FAILURE;
	}
	

	//
	// Extract the driver from the resources
	//
	if (!Sess->ServiceManager.ExtractDriverFromResource())
	{
		xlog(LOG_CRITICAL, L"Failed to extract driver from resource, aborting...\n");
		delete Sess;
		return EXIT_FAILURE;
	}

	xlog(LOG_SUCCESS, L"Driver extracted...\n");


	//
	// Create the frontend listening socket
	//
	
	if (!Sess->FrontEndServer.CreatePipe())
	{
		retcode = EXIT_FAILURE;
		goto __CreateServerPipeFailed;
	}

	xlog(LOG_SUCCESS, L"Named pipe '%s' created...\n", CFB_PIPE_NAME);

	//
	// Create the service and load the driver
	//
	if (!Sess->ServiceManager.LoadDriver())
	{
		retcode = EXIT_FAILURE;
		goto __LoadDriverFailed;
	}

	xlog(LOG_SUCCESS, L"Service '%s' loaded and started\n", CFB_SERVICE_NAME);


	//
	// Start broker <-> driver thread
	//

	if (!StartBackendManagerThread(Sess))
	{
		retcode = EXIT_FAILURE;
		goto __UnsetEnv;
	}

	
	//
	// Start gui <-> broker thread
	//

	if (!StartFrontendManagerThread(Sess))
	{
		retcode = EXIT_FAILURE;
		goto __UnsetEnv;
	}

	xlog(LOG_SUCCESS, L"Threads started with TIDs %d and %d\n", GetThreadId(Sess->m_hFrontendThreadHandle), GetThreadId(Sess->m_hBackendThreadHandle));


	//
	// Start everything
	//
	Sess->Start();
	ResumeThread(Sess->m_hFrontendThreadHandle);
	ResumeThread(Sess->m_hBackendThreadHandle);


	//
	// Wait for those 2 threads to finish
	//
	ThreadHandles[0] = Sess->m_hFrontendThreadHandle;
	ThreadHandles[1] = Sess->m_hBackendThreadHandle;

	dwWaitResult = WaitForMultipleObjects(_countof(ThreadHandles), ThreadHandles, TRUE, INFINITE);

	switch (dwWaitResult)
	{
	case WAIT_OBJECT_0:
		xlog(LOG_SUCCESS, L"All threads ended, cleaning up...\n");
		break;

	default:
		PrintErrorWithFunctionName(L"WaitForMultipleObjects");
		retcode = EXIT_FAILURE;
		break;
	}


__UnsetEnv:
	
	if (!Sess->ServiceManager.UnloadDriver())
	{
		xlog(LOG_CRITICAL, L"A critical error occured while unloading driver\n");
		retcode = EXIT_FAILURE;
	}
	else
	{
		xlog(LOG_SUCCESS, L"Service '%s' unloaded and deleted\n", CFB_SERVICE_NAME);
	}


__LoadDriverFailed:

	//
	// Flush and stop the pipe, then unload the service, and the driver
	//
	if (!Sess->FrontEndServer.ClosePipe())
	{
		xlog(LOG_CRITICAL, L"A critical error occured while closing named pipe\n");
		retcode = EXIT_FAILURE;
	} 
	else
	{
		xlog(LOG_SUCCESS, L"Closed '%s' unloaded and deleted\n", CFB_PIPE_NAME);
	}

	

__CreateServerPipeFailed:

	if (!Sess->ServiceManager.DeleteDriverFromDisk())
	{
		xlog(LOG_CRITICAL, L"A critical error occured while deleting driver from disk\n");
		retcode = EXIT_FAILURE;
	}
	else
	{
		xlog(LOG_SUCCESS, L"Driver deleted\n");
	}

	delete Sess;


#ifdef _DEBUG
	if(retcode != EXIT_SUCCESS)
		xlog(LOG_CRITICAL, L"%s exited with error(s)\n", argv[0]);
#endif // _DEBUG


	return retcode;
}