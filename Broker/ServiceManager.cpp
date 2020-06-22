#include "main.h"

extern Session* Sess;


/*++

Class Description:

This class manages the IrpDumper driver stored in the PE resource section, and 
defines all the functions to properly:

- extract the driver to disk
- create / delete the service
- load / unload the driver
- delete the PE file from disk


CFB can also be installed as a process service to facilitate automation, using sc.exe:
	sc.exe create CFB_Broker binPath= "\path\to\Broker.exe --service" DisplayName= "Furious Beaver process service"
	
Then can be manipulated with the usual `sc start/stop`, and uninstalled with `sc delete`
--*/




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
	if (!ExtractDriverFromResource())
		RAISE_GENERIC_EXCEPTION("ExtractDriverFromResource() failed");

	bIsDriverExtracted = TRUE;

	if (!LoadDriver())
		RAISE_GENERIC_EXCEPTION("LoadDriver() failed");

	bIsDriverLoaded = TRUE;
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
	if (bIsDriverLoaded)
	{
		if (!UnloadDriver())
			RAISE_GENERIC_EXCEPTION("UnloadDriver() failed");
	}

	if (bIsDriverExtracted)
	{
		if (!DeleteDriverFromDisk())
			RAISE_GENERIC_EXCEPTION("DeleteDriverFromDisk() failed");
	}
}


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
	if (!::WriteFile(hDriverFile, hgDriverRsc, dwDriverSize, &dwWritten, NULL))
	{
		PrintErrorWithFunctionName(L"WriteFile()");
		retcode = FALSE;
	}

	::CloseHandle(hDriverFile);
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

	hSCManager = ServiceManagerHandle( 
		::OpenSCManager(L"", SERVICES_ACTIVE_DATABASE, SC_MANAGER_CREATE_SERVICE)
	);

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
	
	hService = ServiceManagerHandle(
		::CreateService(
			hSCManager.Get(),
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
		)
	);

	//
	// if the service was already registered, just open it
	//
	if (!hService)
	{
		if (::GetLastError() != ERROR_SERVICE_EXISTS)
		{
			PrintErrorWithFunctionName(L"CreateService()");
			return FALSE;
		}

		hService = ServiceManagerHandle(
			::OpenService(hSCManager.Get(), CFB_SERVICE_NAME, SERVICE_START | DELETE | SERVICE_STOP)
		);
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

	if (!::StartService(hService.Get(), 0, NULL))
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
	SERVICE_STATUS DriverServiceStatus;

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Stopping service '%s'\n", CFB_SERVICE_NAME);
#endif
	if (!::ControlService(hService.Get(), SERVICE_CONTROL_STOP, &DriverServiceStatus))
	{
		PrintErrorWithFunctionName(L"ControlService");
		return FALSE;
	}

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"Service '%s' stopped\n", CFB_SERVICE_NAME);
#endif
	if (!::DeleteService(hService.Get()))
	{
		PrintErrorWithFunctionName(L"DeleteService");
		return FALSE;
	}


	//::CloseServiceHandle(hService);
	//::CloseServiceHandle(hSCManager);

	return TRUE;
}



static VOID ServiceCtrlHandler(DWORD dwCtrlCode)
{
	ServiceManager& ServiceManager = Sess->ServiceManager;

	switch (dwCtrlCode)
	{
	case SERVICE_CONTROL_CONTINUE:
	case SERVICE_CONTROL_INTERROGATE:
	case SERVICE_CONTROL_PAUSE:
	case SERVICE_CONTROL_SHUTDOWN:
		xlog(LOG_ERROR, L"Unhandled control code: 0x%x\n", dwCtrlCode);
		break;

	case SERVICE_CONTROL_STOP:

		if (ServiceManager.m_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		ServiceManager.m_ServiceStatus.dwControlsAccepted = 0;
		ServiceManager.m_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		ServiceManager.m_ServiceStatus.dwWin32ExitCode = 0;
		ServiceManager.m_ServiceStatus.dwCheckPoint = 4;

		if (::SetServiceStatus(ServiceManager.m_StatusHandle, &ServiceManager.m_ServiceStatus) == FALSE)
		{
			xlog(LOG_DEBUG, L"SetServiceStatus() failed");
			break;
		}
	
		::SetEvent(ServiceManager.m_ServiceStopEvent);
		Sess->Stop();
		break;

	default:
		break;
	}

	return;
}


/*++

Routine Description:

Static routine to initialize the own process service. 


Arguments:

	argc - 

	argv - 


Return Value:

	None

--*/
static VOID ServiceMain(DWORD argc, LPWSTR* argv)
{
	ServiceManager& ServiceManager = Sess->ServiceManager;

	do
	{

		ServiceManager.m_StatusHandle = ::RegisterServiceCtrlHandler(WIN32_SERVICE_NAME, ServiceCtrlHandler);
		if (!ServiceManager.m_StatusHandle)
		{
			xlog(LOG_ERROR, L"RegisterServiceCtrlHandler() failed\n");
			break;
		}

		::ZeroMemory(&ServiceManager.m_ServiceStatus, sizeof(ServiceManager.m_ServiceStatus));
		ServiceManager.m_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
		ServiceManager.m_ServiceStatus.dwControlsAccepted = 0;
		ServiceManager.m_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
		ServiceManager.m_ServiceStatus.dwWin32ExitCode = 0;
		ServiceManager.m_ServiceStatus.dwServiceSpecificExitCode = 0;
		ServiceManager.m_ServiceStatus.dwCheckPoint = 0;

		if (::SetServiceStatus(ServiceManager.m_StatusHandle, &ServiceManager.m_ServiceStatus) == FALSE)
		{
			xlog(LOG_ERROR, L"SetServiceStatus() failed\n");
			break;
		}


		ServiceManager.m_ServiceStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (ServiceManager.m_ServiceStopEvent == NULL)
		{
			ServiceManager.m_ServiceStatus.dwControlsAccepted = 0;
			ServiceManager.m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
			ServiceManager.m_ServiceStatus.dwWin32ExitCode = ::GetLastError();
			ServiceManager.m_ServiceStatus.dwCheckPoint = 1;

			if (::SetServiceStatus(ServiceManager.m_StatusHandle, &ServiceManager.m_ServiceStatus) == FALSE)
			{
				xlog(LOG_ERROR, L"SetServiceStatus() failed\n");
			}
			break;
		}


		ServiceManager.m_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
		ServiceManager.m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		ServiceManager.m_ServiceStatus.dwWin32ExitCode = 0;
		ServiceManager.m_ServiceStatus.dwCheckPoint = 0;

		if (::SetServiceStatus(ServiceManager.m_StatusHandle, &ServiceManager.m_ServiceStatus) == FALSE)
		{
			xlog(LOG_ERROR, L"SetServiceStatus() failed\n");
			break;
		}

		dbg(L"CFB background service ready, starting thread...\n");


		//
		// Let's start CFB in background
		//

		HANDLE hThread = ::CreateThread(NULL, 0, RunForever, &ServiceManager.m_ServiceStopEvent, 0, NULL);
		if (!hThread)
		{
			xlog(LOG_ERROR, L"Failed to start the RunForever() thread\n");
			break;
		}

		::WaitForSingleObject(hThread, INFINITE);
		
		::CloseHandle(ServiceManager.m_ServiceStopEvent);

		//
		// Propagate the stop info to the service controller
		//
		ServiceManager.m_ServiceStatus.dwControlsAccepted = 0;
		ServiceManager.m_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		ServiceManager.m_ServiceStatus.dwWin32ExitCode = 0;
		ServiceManager.m_ServiceStatus.dwCheckPoint = 3;

		if (::SetServiceStatus(ServiceManager.m_StatusHandle, &ServiceManager.m_ServiceStatus) == FALSE)
		{
			xlog(LOG_ERROR, L"SetServiceStatus() failed\n");
		}


		//
		// Delete the session (i.e. stop the process service and unload the driver)
		//
		delete Sess;

	} 
	while (0);


	return;
}


/*++

Routine Description:

Static routine to register the own process service.


Arguments:

	None


Return Value:

	None

--*/
BOOL ServiceManager::RegisterService()
{
	auto lpswServiceName = (LPWSTR)WIN32_SERVICE_NAME;

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{lpswServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		{NULL, NULL}
	};

	return ::StartServiceCtrlDispatcher(ServiceTable);
}