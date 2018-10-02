#ifndef __HOOKED_DRIVERS_H__
#define __HOOKED_DRIVERS_H__

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"


#pragma once

typedef struct __hooked_driver
{
	BOOLEAN Enabled;
	WCHAR Name[HOOKED_DRIVER_MAX_NAME_LEN];
	UNICODE_STRING UnicodeName;
	PDRIVER_OBJECT DriverObject;
	PVOID OldDeviceControlRoutine;
	PVOID OldReadRoutine;
	PVOID OldWriteRoutine;
	struct __hooked_driver *Next;
}
HOOKED_DRIVER, *PHOOKED_DRIVER;

PHOOKED_DRIVER g_HookedDriversHead;


UINT32 GetNumberOfHookedDrivers();
PHOOKED_DRIVER GetLastHookedDriver();
PHOOKED_DRIVER GetPreviousHookedDriver(PHOOKED_DRIVER pDriver);
PHOOKED_DRIVER GetNextHookedDriver(PHOOKED_DRIVER pDriver);
BOOLEAN IsDriverHooked(PDRIVER_OBJECT pObj);
PHOOKED_DRIVER GetHookedDriverByName(LPWSTR lpDriverName);
PHOOKED_DRIVER GetHookedDriverByIndex(UINT32 dwIndex);


#endif /* __HOOKED_DRIVERS_H__ */