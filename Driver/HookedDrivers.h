#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"


typedef struct _HOOKED_DRIVER
{
	BOOLEAN Enabled;
	WCHAR Name[HOOKED_DRIVER_MAX_NAME_LEN];
	UNICODE_STRING UnicodeName;
	PDRIVER_OBJECT DriverObject;
	LIST_ENTRY ListEntry;
	PDRIVER_DISPATCH OriginalRoutines[IRP_MJ_MAXIMUM_FUNCTION+1];
	PFAST_IO_READ FastIoRead;
	PFAST_IO_WRITE FastIoWrite;
	PFAST_IO_DEVICE_CONTROL FastIoDeviceControl;
	UINT64 NumberOfRequestIntercepted;
}
HOOKED_DRIVER, *PHOOKED_DRIVER;


#define ENABLED_DRIVERS_ONLY 1

void 
InitializeHookedDriverStructures();

UINT32 
GetNumberOfHookedDrivers();

NTSTATUS 
GetNamesOfHookedDrivers(_In_ UCHAR Flags, _Out_ PWCHAR lpwsOutputBuffer, _In_ ULONG ulOutputBufferSize, _Out_ PULONG pdwDataWritten);

BOOLEAN 
IsDriverHooked(_In_ PDRIVER_OBJECT pDriverName);

NTSTATUS 
GetHookedDriverByName(_In_ LPWSTR lpDriverName, _Out_ PHOOKED_DRIVER* pHookedDrv);

PHOOKED_DRIVER 
GetHookedDriverFromDeviceObject(_In_ PDEVICE_OBJECT DeviceObject);
