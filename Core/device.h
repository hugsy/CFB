#pragma once


BOOL OpenCfbDevice();
BOOL CloseCfbDevice();

BOOL HookDriver(LPWSTR lpDriver);
BOOL UnhookDriver(LPWSTR lpDriver);
BOOL GetNumberOfDrivers(PDWORD pdwNbDrivers);
BOOL GetHookedDriverInfo(DWORD dwDriverIndex, PHOOKED_DRIVER_INFO hDrvInfo);
BOOL ReadCfbDevice( LPVOID Buffer, DWORD BufSize, LPDWORD lpNbBytesRead );
BOOL SetEventNotificationHandle( HANDLE hEvent );