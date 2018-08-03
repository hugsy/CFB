#pragma once


BOOL OpenCfbDevice();
BOOL CloseCfbDevice();


BOOL EnumerateHookedDrivers();
BOOL HookDriver(LPWSTR lpDriver);
BOOL UnhookDriver(LPWSTR lpDriver);