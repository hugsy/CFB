#include "common.h"

#include <wil\resource.h>
#include "resource.h"
#include "CfbException.h"


#define CFB_DRIVER_LOCATION_DIRECTORY L"C:\\Windows\\System32\\Drivers"
#define WIN32_SERVICE_NAME L"CFB_Broker"

#define GetDriverOnDiskFullPath(x){\
 	wcscat_s(x, MAX_PATH, CFB_DRIVER_LOCATION_DIRECTORY);\
	wcscat_s(x, MAX_PATH, L"\\");\
	wcscat_s(x, MAX_PATH, CFB_DRIVER_NAME);\
 }


class ServiceManager 
{
public:
	ServiceManager() noexcept(false);
	~ServiceManager() noexcept(false);
	
	BOOL RegisterService();
	

	SERVICE_STATUS m_ServiceStatus = { 0 };
	SERVICE_STATUS_HANDLE m_StatusHandle = NULL;
	HANDLE m_ServiceStopEvent = INVALID_HANDLE_VALUE;
	BOOL bRunInBackground = FALSE;


private:
	BOOL ExtractDriverFromResource();
	BOOL LoadDriver();

	BOOL UnloadDriver();
	BOOL DeleteDriverFromDisk();


	SC_HANDLE hService = NULL;
	SC_HANDLE hSCManager = NULL;

	BOOL bIsDriverLoaded = FALSE;
	BOOL bIsDriverExtracted = FALSE;

};
