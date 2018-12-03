#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"



typedef struct 
{
	BOOLEAN Enabled;
	WCHAR Name[HOOKED_DRIVER_MAX_NAME_LEN];
	UNICODE_STRING UnicodeName;
	PDRIVER_OBJECT DriverObject;
    /*
	PVOID OldDeviceControlRoutine;
	PVOID OldReadRoutine;
	PVOID OldWriteRoutine;
    */
	LIST_ENTRY ListEntry;
	PVOID OriginalRoutines[IRP_MJ_MAXIMUM_FUNCTION+1];
}
HOOKED_DRIVER, *PHOOKED_DRIVER;



void InitializeHookedDriverStructures();
UINT32 GetNumberOfHookedDrivers();
BOOLEAN IsDriverHooked(IN PDRIVER_OBJECT pDriverName);
NTSTATUS GetHookedDriverByName(IN LPWSTR lpDriverName, OUT PHOOKED_DRIVER *pHookedDriver);
