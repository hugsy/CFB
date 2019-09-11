#include "driver.h"




/*++

Routine Description:

Extracts the driver from the resources, and dump it to the location defined by
`CFB_DRIVER_LOCATION_DIRECTORY` in drivers.h


Arguments:

	None


Return Value:
	Returns TRUE upon successful extraction of the driver from the resource of the PE 
	file, FALSE if any error occured.

--*/
BOOL ExtractDriverFromResource()
{
	BOOL retcode = TRUE;

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Extracting driver from resources\n");
#endif

	HRSRC DriverRsc = FindResource(NULL, MAKEINTRESOURCE(IDR_CFB_DRIVER1), L"CFB_DRIVER");
	if (!DriverRsc)
	{
		PrintErrorWithFunctionName(L"FindResource()");
		return FALSE;
	}

	DWORD dwDriverSize = SizeofResource(NULL, DriverRsc);
	if (!dwDriverSize)
	{
		PrintErrorWithFunctionName(L"SizeofResource()");
		return FALSE;
	}

	HGLOBAL hgDriverRsc = LoadResource(NULL, DriverRsc);
	if (!hgDriverRsc)
	{
		PrintErrorWithFunctionName(L"LoadResource()");
		return FALSE;
	}


#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Writing driver to '%s'\n", CFB_DRIVER_LOCATION_DIRECTORY);
#endif

	WCHAR lpszFilePath[MAX_PATH] = { 0, };
	GetDriverOnDiskFullPath(lpszFilePath);

	HANDLE hDriverFile = CreateFile(lpszFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDriverFile == INVALID_HANDLE_VALUE)
	{
		PrintErrorWithFunctionName(L"CreateFile()");
		return FALSE;
	}

	DWORD dwWritten;
	if (!WriteFile(hDriverFile, hgDriverRsc, dwDriverSize, &dwWritten, NULL))
	{
		PrintErrorWithFunctionName(L"WriteFile()");
		retcode = FALSE;
	}

	CloseHandle(hDriverFile);
	return retcode;
}


/*++

Routine Description:

Delete the driver extracted from the PE resources from the disk.


Arguments:

	None


Return Value:

	Returns TRUE upon successful deletion of the driver from the disk.

--*/
BOOL DeleteDriverFromDisk()
{
	WCHAR lpszFilePath[MAX_PATH] = { 0, };
	GetDriverOnDiskFullPath(lpszFilePath);
	return DeleteFile(lpszFilePath);
}


/*++

Routine Description:

Creates and starts a service for the driver.


Arguments:

	None


Return Value:

	Returns TRUE if the service was successfully created, and the driver loaded;
	FALSE in any other case

--*/
BOOL LoadDriver()
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Loading '%s'\n", CFB_DRIVER_NAME);
#endif

	g_hSCManager = OpenSCManager(L"", SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE);

	if (!g_hSCManager)
	{
		PrintErrorWithFunctionName(L"OpenSCManager()");
		return FALSE;
	}

	WCHAR lpPath[MAX_PATH] = { 0, };
	GetDriverOnDiskFullPath(lpPath);

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Creating the service '%s' for kernel driver '%s'\n", CFB_SERVICE_NAME, lpPath);
#endif 
	g_hService = CreateService(
		g_hSCManager,
		CFB_SERVICE_NAME,
		CFB_SERVICE_DESCRIPTION,
		SERVICE_START | DELETE | SERVICE_STOP,
		SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_IGNORE,
		lpPath,
		NULL, 
		NULL, 
		NULL, 
		NULL, 
		NULL
	);

	//
	// if the service was already registered, just open it
	//
	if (!g_hService)
	{
		if (GetLastError() != ERROR_SERVICE_EXISTS)
		{
			PrintErrorWithFunctionName(L"CreateService()");
			return FALSE;
		}

		g_hService = OpenService(g_hSCManager, CFB_SERVICE_NAME, SERVICE_START | DELETE | SERVICE_STOP);
		if (!g_hService)
		{
			PrintErrorWithFunctionName(L"CreateService()");
			return FALSE;
		}
	}

	//
	// start the service
	//
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Starting service '%s'\n", CFB_SERVICE_NAME);
#endif 

	if (!StartService(g_hService, 0, NULL))
	{
		PrintErrorWithFunctionName(L"StartService()");
		return FALSE;
	}

	return TRUE;
}


/*++

Routine Description:

Unloads the driver.


Arguments:

	None


Return Value:

	Returns TRUE if the driver was successfully unloaded; FALSE in any other case

--*/
BOOL UnloadDriver()
{
	SERVICE_STATUS ServiceStatus;
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Stopping service '%s'\n", CFB_SERVICE_NAME);
#endif
	if (!ControlService(g_hService, SERVICE_CONTROL_STOP, &ServiceStatus))
	{
		PrintErrorWithFunctionName(L"ControlService");
		return FALSE;
	}

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Service '%s' stopped\n", CFB_SERVICE_NAME);
#endif
	if (!DeleteService(g_hService))
	{
		PrintErrorWithFunctionName(L"DeleteService");
		return FALSE;
	}


	CloseServiceHandle(g_hService);
	CloseServiceHandle(g_hSCManager);

	return TRUE;
}


/*++

Routine Description:

Reads the new tasks received by the FrontEnd thread, and bounces thoses requests to the backend (i.e. the driver) 
via the IOCTL codes (which can be found in Driver\Header Files\IoctlCodes.h).

The driver must acknowledge the request by sending a response (even if the result content is asynchronous).


Arguments:

	None


Return Value:
	
	Returns 0

--*/
static DWORD BackendConnectionHandlingThread(_In_ LPVOID /* lpParameter */)
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Getting a handle to the device object\n");
#endif

	wil::unique_handle hDevice(
		::CreateFile(
			CFB_USER_DEVICE_NAME,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL
		)
	);

	if (!hDevice)
	{
		PrintErrorWithFunctionName(L"CreateFile(g_hDevice");
		return GetLastError();
	}

	
	extern TaskManager g_RequestManager, g_ResponseManager;

	while (g_bIsRunning)
	{
		//
		// blocking-pop from request task list
		//

		auto in_task = g_RequestManager.pop();
		

		//
		// flag task as Delivered
		//
		in_task.SetState(TaskState::Delivered);


		//
		// send the DeviceIoControl
		//
		xlog(LOG_DEBUG, L"Type: %s, Length: %d\n", in_task.Type(), in_task.Length());

		
		byte* lpOutputBuffer = nullptr;
		DWORD dwOutputBufferSize = 0;
		DWORD dwNbBytesReturned = 0;

		while (TRUE)
		{
			BOOL bRes = ::DeviceIoControl(
				hDevice.get(),
				in_task.IoctlCode(),
				in_task.Data(),
				in_task.Length(),
				lpOutputBuffer,
				dwOutputBufferSize,
				&dwNbBytesReturned,
				NULL
			);

			//
			// If the ioctl was ok, we exit
			//
			if (bRes)
				break;

			DWORD dwErrCode = ::GetLastError();

			//
			// If the buffer was too small, retry with the appropriate size
			//
			if (dwErrCode == ERROR_INSUFFICIENT_BUFFER)
			{
				dwOutputBufferSize = dwNbBytesReturned;
				if (lpOutputBuffer)
					delete[] lpOutputBuffer;

				lpOutputBuffer = new byte[dwOutputBufferSize];
				continue;
			}

			break;
		}
		

		//
		// flag task as Completed
		//
		in_task.SetState(TaskState::Completed);

		delete& in_task;


		//
		// Prepare the response task
		//
		Task out_task(TaskType::IoctlResponse, lpOutputBuffer, dwOutputBufferSize, ::GetLastError());
		delete[] lpOutputBuffer;


		//
		// push to response task list
		//
		g_ResponseManager.push(out_task);

		out_task.SetState(TaskState::Queued);
	}


	return 0;
}



/*++

Routine Description:

Start the thread responsible for the communcation between the broker and the backend (i.e.
the driver).


Arguments:

	lpThread -  a pointer to the handle of the created


Return Value:

	Returns TRUE if the thread was successfully created; FALSE in any other case

--*/
_Success_(return)
BOOL StartBackendManagerThread(_Out_ PHANDLE lpThread)
{
	DWORD dwThreadId;

	HANDLE hThread = CreateThread(
		NULL,
		0,
		BackendConnectionHandlingThread,
		NULL,
		0,
		&dwThreadId
	);

	if (!hThread)
	{
		PrintError(L"CreateThread(Driver)");
		return FALSE;
	}

#ifdef _DEBUG
	xlog(LOG_DEBUG, "CreateThread(Driver) started as TID=%d\n", dwThreadId);
#endif

	*lpThread = hThread;

	return TRUE;
}