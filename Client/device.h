#pragma once


BOOL OpenCfbDevice();
BOOL CloseCfbDevice();

BOOL HookDriver(LPWSTR lpDriver);
BOOL UnhookDriver(LPWSTR lpDriver);
BOOL GetNumberOfDrivers(PDWORD pdwNbDrivers);
BOOL GetHookedDriverInfo(DWORD dwDriverIndex);
