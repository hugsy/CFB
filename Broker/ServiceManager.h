#include "common.h"

#include "resource.h"
#include "CfbException.h"
#include "SafeHandle.h"


#define CFB_DRIVER_LOCATION_DIRECTORY L"C:\\Windows\\System32\\Drivers"
#define WIN32_SERVICE_NAME L"CFB_Broker"

#define GetDriverOnDiskFullPath(x){\
 	wcscat_s(x, MAX_PATH, CFB_DRIVER_LOCATION_DIRECTORY);\
	wcscat_s(x, MAX_PATH, L"\\");\
	wcscat_s(x, MAX_PATH, CFB_DRIVER_NAME);\
 }


class ServiceManagerHandle : public GenericHandle<SC_HANDLE>
{
public:
	using GenericHandle<SC_HANDLE>::GenericHandle;

	void Close() override
	{
		if (bool(_h))
		{
			::CloseServiceHandle(_h);
			_h = nullptr;
		}
	}
};


class ServiceManager 
{
public:
	ServiceManager() noexcept(false);
	~ServiceManager() noexcept(false);
	
	BOOL RegisterService();
	

	SERVICE_STATUS m_ServiceStatus = { 0 };
	SERVICE_STATUS_HANDLE m_StatusHandle = nullptr;
	HANDLE m_ServiceStopEvent = INVALID_HANDLE_VALUE;
	BOOL bRunInBackground = FALSE;


private:
	BOOL ExtractDriverFromResource();
	BOOL LoadDriver();

	BOOL UnloadDriver();
	BOOL DeleteDriverFromDisk();


	ServiceManagerHandle hService = nullptr;
	ServiceManagerHandle hSCManager = nullptr;

	BOOL bIsDriverLoaded = FALSE;
	BOOL bIsDriverExtracted = FALSE;

};


