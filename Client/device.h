#pragma once


HANDLE OpenCfbDevice();
BOOLEAN CloseCfbDevice();
BOOLEAN EnumerateHookedDrivers();
NTSTATUS HookDriver(LPWSTR lpDriver);
NTSTATUS UnhookDriver(LPWSTR lpDriver);