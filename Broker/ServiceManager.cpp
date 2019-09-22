#include "ServiceManager.h"

/*++

Class Description:

This class manages the IrpDumper driver stored in the PE resource section, and 
defines all the functions to properly:

- extract the driver to disk
- create / delete the service
- load / unload the driver
- delete the PE file from disk


--*/





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
BOOL ServiceManager::ExtractDriverFromResource()
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
BOOL ServiceManager::DeleteDriverFromDisk()
{
	WCHAR lpszFilePath[MAX_PATH] = { 0, };
	GetDriverOnDiskFullPath(lpszFilePath);
	return DeleteFile(lpszFilePath);
}


/*++

Routine Description:

Creates the service manager object.


Arguments:

	None


Return Value:
	Nothing, throw an exception on any error

--*/
ServiceManager::ServiceManager()
{
	if (!LoadDriver())
		RAISE_GENERIC_EXCEPTION("LoadDriver() failed");

}



/*++

Routine Description:

Destroy the service manager object.


Arguments:

	None


Return Value:
	Nothing, throw an exception on any error
	
--*/
ServiceManager::~ServiceManager() noexcept(false)
{
	if (!UnloadDriver())
		RAISE_GENERIC_EXCEPTION("UnloadDriver() failed");

}


/*++

Routine Description:

Creates and starts a service for the IrpDumper driver.


Arguments:

	None


Return Value:

	Returns TRUE if the service was successfully created, and the driver loaded;
	FALSE in any other case

--*/
BOOL ServiceManager::LoadDriver()
{
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Loading '%s'\n", CFB_DRIVER_NAME);
#endif

	hSCManager = OpenSCManager(L"", SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE);

	if (!hSCManager)
	{
		PrintErrorWithFunctionName(L"OpenSCManager()");
		return FALSE;
	}

	WCHAR lpPath[MAX_PATH] = { 0, };
	GetDriverOnDiskFullPath(lpPath);

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Creating the service '%s' for kernel driver '%s'\n", CFB_SERVICE_NAME, lpPath);
#endif 
	
	hService = CreateService(
		hSCManager,
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
	if (!hService)
	{
		if (GetLastError() != ERROR_SERVICE_EXISTS)
		{
			PrintErrorWithFunctionName(L"CreateService()");
			return FALSE;
		}

		hService = OpenService(hSCManager, CFB_SERVICE_NAME, SERVICE_START | DELETE | SERVICE_STOP);
		if (!hSCManager)
		{
			PrintErrorWithFunctionName(L"OpenService()");
			return FALSE;
		}
	}

	//
	// start the service
	//
#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Starting service '%s'\n", CFB_SERVICE_NAME);
#endif 

	if (!StartService(hService, 0, NULL))
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
BOOL ServiceManager::UnloadDriver()
{
	SERVICE_STATUS ServiceStatus;

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Stopping service '%s'\n", CFB_SERVICE_NAME);
#endif
	if (!ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus))
	{
		PrintErrorWithFunctionName(L"ControlService");
		return FALSE;
	}

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Service '%s' stopped\n", CFB_SERVICE_NAME);
#endif
	if (!DeleteService(hService))
	{
		PrintErrorWithFunctionName(L"DeleteService");
		return FALSE;
	}


	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);

	return TRUE;
}
