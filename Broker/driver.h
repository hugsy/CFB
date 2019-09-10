#pragma once

#include <memory>
#include <wil/resource.h>


#include "resource.h"

#include "common.h"
#include "main.h"


#define CFB_DRIVER_LOCATION_DIRECTORY L"C:\\Windows\\System32\\Drivers"

#define GetDriverOnDiskFullPath(x){\
 	wcscat_s(x, MAX_PATH, CFB_DRIVER_LOCATION_DIRECTORY);\
	wcscat_s(x, MAX_PATH, L"\\");\
	wcscat_s(x, MAX_PATH, CFB_DRIVER_NAME);\
 }


static HANDLE g_hDevice = NULL;
static SC_HANDLE g_hService = NULL;
static SC_HANDLE g_hSCManager = NULL;


BOOL ExtractDriverFromResource();
BOOL LoadDriver();
BOOL UnloadDriver();
BOOL DeleteDriverFromDisk();
_Success_(return) BOOL StartDriverThread(_Out_ PHANDLE lpThread);
