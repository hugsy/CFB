#pragma once

#include <windows.h>
#include <wchar.h>

#include "../Common/common.h"
#include "../Driver/IoctlCodes.h"


//
// Internal function signatures
//
BOOL OpenCfbDevice();
BOOL CloseCfbDevice();
BOOL GetNumberOfDrivers(PDWORD pdwNbDrivers);
BOOL GetHookedDriverInfo(DWORD dwDriverIndex, PHOOKED_DRIVER_INFO hDrvInfo);


//
// Exported functions
//
__declspec(dllexport) BOOL ReadCfbDevice(LPVOID Buffer, DWORD BufSize, LPDWORD lpNbBytesRead);
__declspec(dllexport) DWORD GetCfbMessageHeaderSize();
__declspec(dllexport) BOOL HookDriver(IN LPWSTR lpDriverName);
__declspec(dllexport) BOOL UnhookDriver(IN LPWSTR lpDriverName);
__declspec(dllexport) BOOL SetEventNotificationHandle(IN HANDLE hEvent);
__declspec(dllexport) BOOL EnableMonitoring();
__declspec(dllexport) BOOL DisableMonitoring();
