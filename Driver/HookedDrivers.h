#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"

enum _FAST_IO_FUNCTION
{
	FASTIO_DEVICE_CONTROL = 0,
	FASTIO_READ,
	FASTIO_WRITE,
	FASTIO_MAXIMUM_FUNCTION
};

typedef struct 
{
	BOOLEAN Enabled;
	WCHAR Name[HOOKED_DRIVER_MAX_NAME_LEN];
	UNICODE_STRING UnicodeName;
	PDRIVER_OBJECT DriverObject;
	LIST_ENTRY ListEntry;
	PVOID _Function_class_(DRIVER_DISPATCH) OriginalRoutines[IRP_MJ_MAXIMUM_FUNCTION+1];
	PVOID OriginalFastIoDispatch[FASTIO_MAXIMUM_FUNCTION];
}
HOOKED_DRIVER, *PHOOKED_DRIVER;


void InitializeHookedDriverStructures();
UINT32 GetNumberOfHookedDrivers();
BOOLEAN IsDriverHooked(IN PDRIVER_OBJECT pDriverName);
NTSTATUS GetHookedDriverByName(IN LPWSTR lpDriverName, OUT PHOOKED_DRIVER *pHookedDriver);
