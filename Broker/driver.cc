#include "driver.h"




/*++

Extracts the driver from the resources, and dump it to the location defined by 
`CFB_DRIVER_LOCATION_DIRECTORY` in drivers.h

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
		PrintError(L"ExtractDriverFromResource:FindResource() failed");
		return FALSE;
	}

	DWORD dwDriverSize = SizeofResource(NULL, DriverRsc);
	if (!dwDriverSize)
	{
		PrintError(L"ExtractDriverFromResource:SizeofResource() failed");
		return FALSE;
	}

	HGLOBAL hgDriverRsc = LoadResource(NULL, DriverRsc);
	if (!hgDriverRsc)
	{
		PrintError(L"ExtractDriverFromResource:LoadResource() failed");
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
		PrintError(L"CreateFile()");
		return FALSE;
	}

	DWORD dwWritten;
	if (!WriteFile(hDriverFile, hgDriverRsc, dwDriverSize, &dwWritten, NULL))
	{
		PrintError(L"WriteFile()");
		retcode = FALSE;
	}

	CloseHandle(hDriverFile);
	return retcode;
}


/*++

--*/
BOOL DeleteDriverFromDisk()
{
	WCHAR lpszFilePath[MAX_PATH] = { 0, };
	GetDriverOnDiskFullPath(lpszFilePath);
	return DeleteFile(lpszFilePath);
}


/*++

Creates and starts a service for the driver.

--*/
BOOL LoadDriver()
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Loading '%s'\n", CFB_DRIVER_NAME);
#endif

	g_hSCManager = OpenSCManager(L"", SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE);

	if (!g_hSCManager)
	{
		PrintError(L"OpenSCManager()");
		return FALSE;
	}

	WCHAR lpPath[MAX_PATH] = { 0, };
	GetDriverOnDiskFullPath(lpPath);

	xlog(LOG_DEBUG, L"Create the service '%s' for kernel driver '%s'\n", CFB_SERVICE_NAME, lpPath);

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
			PrintError(L"CreateService()");
			return FALSE;
		}

		g_hService = OpenService(g_hSCManager, CFB_SERVICE_NAME, SERVICE_START | DELETE | SERVICE_STOP);
		if (!g_hService)
		{
			PrintError(L"CreateService()");
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
		PrintError(L"StartService()");
		return FALSE;
	}

	return TRUE;
}


/*++

Stops and unloads the service to the driver.

--*/
BOOL UnloadDriver()
{
	SERVICE_STATUS ServiceStatus;
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Stopping service '%s'\n", CFB_SERVICE_NAME);
#endif
	if (!ControlService(g_hService, SERVICE_CONTROL_STOP, &ServiceStatus))
	{
		PrintError(L"ControlService");
		return FALSE;
	}

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Service '%s' stopped\n", CFB_SERVICE_NAME);
#endif
	if (!DeleteService(g_hService))
	{
		PrintError(L"DeleteService");
		return FALSE;
	}


	CloseServiceHandle(g_hService);
	CloseServiceHandle(g_hSCManager);

	return TRUE;
}


/*++

--*/
static DWORD DriverThread(_In_ LPVOID lpParameter)
{
	while (TRUE)
	{
		Sleep(10 * 1000);
	}

	return 0;
}



/*++

--*/
_Success_(return)
BOOL StartDriverThread(_Out_ PHANDLE lpThread)
{
	DWORD dwThreadId;

	HANDLE hThread = CreateThread(
		NULL,
		0,
		DriverThread,
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