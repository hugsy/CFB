/*++

This is the main file for the broker.

The broker is a privileged process (must have at least SeDebug and SeLoadDriver) and is responsible 
for communicating with the IrpDumper driver by:
- create the service and load the driver
- instructing the driver to add/remove hooks to specific driver(s)
- fetching IRP data from the hooked drivers
- upon cleanup the resources, unload the driver and delete the service.

--*/


#include "main.h"



/*++

Check that we have sufficient privileges.

--*/
static BOOL RunInitializationChecks(wchar_t* lpszProgramName)
{
	LUID Luid[2] = { 0, };
	HANDLE hToken = INVALID_HANDLE_VALUE;
	BOOL bRes, bHasPriv;

	xlog(LOG_DEBUG, L"Checking for privileges...\n");

	do {

		bRes = LookupPrivilegeValue(NULL, L"SeDebugPrivilege", &Luid[0]);
		if (!bRes)
			break;

		bRes = LookupPrivilegeValue(NULL, L"SeLoadDriverPrivilege", &Luid[1]);
		if (!bRes)
			break;

		LUID_AND_ATTRIBUTES DebugPrivilege = { 0 };
		DebugPrivilege.Luid = Luid[0];
		DebugPrivilege.Attributes = SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT;

		LUID_AND_ATTRIBUTES LoadDriverPrivilege = { 0 };
		LoadDriverPrivilege.Luid = Luid[1];
		LoadDriverPrivilege.Attributes = SE_PRIVILEGE_ENABLED | SE_PRIVILEGE_ENABLED_BY_DEFAULT;

		PRIVILEGE_SET* PrivSet = (PRIVILEGE_SET*)_malloca(sizeof(PRIVILEGE_SET) * 2);
		PrivSet->PrivilegeCount = 2;
		PrivSet->Privilege[0] = DebugPrivilege;
		PrivSet->Privilege[1] = LoadDriverPrivilege;

		bRes = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
		if (!bRes)
			break;

		bRes = PrivilegeCheck(hToken, PrivSet, &bHasPriv);
		if (!bRes)
			break;
		
		bRes = TRUE;
	}
	while (0);

	if (!bRes)
	{
		if (hToken != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hToken);
		}
	}
	
	return bHasPriv;
}



/*++

The entrypoint for the loader.

--*/
int main(int argc, wchar_t** argv)
{
	int retcode = EXIT_SUCCESS;
	HANDLE hDriver = INVALID_HANDLE_VALUE;
	HANDLE hGui = INVALID_HANDLE_VALUE;
	HANDLE Handles[2] = { 0 };

	xlog(LOG_INFO, L"Starting %s (part of %s (v%.02f) - by <%s>)\n", argv[0], CFB_PROGRAM_NAME_SHORT, CFB_VERSION, CFB_AUTHOR);

#ifdef _DEBUG
	xlog(LOG_DEBUG, L"DEBUG mode on\n");
#endif

	//
	// Check the privileges: the broker must have SeLoadDriverPrivilege and SeDebugPrivilege
	//
	if (!RunInitializationChecks(argv[0]))
	{
		return EXIT_FAILURE;
	}

	xlog(LOG_SUCCESS, L"Privilege check succeeded...\n");


	//
	// Extract the driver from the resources
	//
	if (!ExtractDriverFromResource())
	{
		xlog(LOG_CRITICAL, L"Failed to extract driver from resource, aborting...\n");
		return EXIT_FAILURE;
	}

	xlog(LOG_SUCCESS, L"Driver extracted...\n");


	//
	// Create the named pipe the GUI will read data from
	//
	if (!CreateServerPipe())
	{
		retcode = EXIT_FAILURE;
		goto __CreateServerPipeFailed;
	}

	xlog(LOG_SUCCESS, L"Named pipe '%s' created...\n", CFB_PIPE_NAME);

	//
	// Create the service and load the driver
	//
	if (!LoadDriver())
	{
		retcode = EXIT_FAILURE;
		goto __LoadDriverFailed;
	}

	xlog(LOG_SUCCESS, L"Service '%s' loaded and started\n", CFB_SERVICE_NAME);


	//
	// Start broker <-> driver thread
	//

	if (!StartDriverThread(&hDriver) || hDriver == INVALID_HANDLE_VALUE)
	{
		retcode = EXIT_FAILURE;
		goto __UnsetEnv;
	}

	
	//
	// Start gui <-> broker thread
	//

	if (!StartGuiThread(&hGui) || hGui == INVALID_HANDLE_VALUE)
	{
		retcode = EXIT_FAILURE;
		goto __UnsetEnv;
	}

	xlog(LOG_SUCCESS, L"Threads started\n");


	//
	// Wait for those 2 threads to finish
	//
	Handles[0] = hGui;
	Handles[1] = hDriver;

	WaitForMultipleObjects(2, Handles, TRUE, INFINITE);


__UnsetEnv:
	
	if (!UnloadDriver())
	{
		xlog(LOG_CRITICAL, L"A critical error occured while unloading driver\n");
	}
	else
	{
		xlog(LOG_SUCCESS, L"Service '%s' unloaded and deleted\n", CFB_SERVICE_NAME);
	}


__LoadDriverFailed:

	//
	// Flush and stop the pipe, then unload the service, and the driver
	//
	if (!CloseServerPipe())
	{
		xlog(LOG_CRITICAL, L"A critical error occured while closing named pipe\n");
	} 
	else
	{
		xlog(LOG_SUCCESS, L"Closed '%s' unloaded and deleted\n", CFB_PIPE_NAME);
	}

	

__CreateServerPipeFailed:

	if (!DeleteDriverFromDisk())
	{
		xlog(LOG_CRITICAL, L"A critical error occured while deleting driver from disk\n");
	}
	else
	{
		xlog(LOG_SUCCESS, L"Driver deleted\n");
	}

	return retcode;
}