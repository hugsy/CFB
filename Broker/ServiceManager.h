#include "common.h"

#include <wil\resource.h>
#include "resource.h"


#define CFB_DRIVER_LOCATION_DIRECTORY L"C:\\Windows\\System32\\Drivers"

#define GetDriverOnDiskFullPath(x){\
 	wcscat_s(x, MAX_PATH, CFB_DRIVER_LOCATION_DIRECTORY);\
	wcscat_s(x, MAX_PATH, L"\\");\
	wcscat_s(x, MAX_PATH, CFB_DRIVER_NAME);\
 }


class ServiceManager 
{
public:
	BOOL ExtractDriverFromResource();
	BOOL LoadDriver();
	BOOL UnloadDriver();
	BOOL DeleteDriverFromDisk();


private:
	SC_HANDLE hService = NULL;
	SC_HANDLE hSCManager = NULL;
};
