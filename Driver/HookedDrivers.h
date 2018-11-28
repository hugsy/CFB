#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"



typedef struct __hooked_driver
{
	BOOLEAN Enabled;
	WCHAR Name[HOOKED_DRIVER_MAX_NAME_LEN];
	UNICODE_STRING UnicodeName;
	PDRIVER_OBJECT DriverObject;
	PVOID OldDeviceControlRoutine;
	PVOID OldReadRoutine;
	PVOID OldWriteRoutine;
	LIST_ENTRY ListEntry;
}
HOOKED_DRIVER, *PHOOKED_DRIVER;



void InitializeHookedDriverStructures();
UINT32 GetNumberOfHookedDrivers();
BOOLEAN IsDriverHooked(PDRIVER_OBJECT pObj);
PHOOKED_DRIVER GetHookedDriverByName(LPWSTR lpDriverName);
