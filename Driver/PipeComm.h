#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"
#include "Queue.h"
#include "HookedDrivers.h"
#include "IoSetEventPointer.h"

typedef struct 
{
	UINT32 Pid;
	UINT32 Tid;
	UINT32 IoctlCode;
	UINT32 Type;
	WCHAR DriverName[MAX_PATH];
	WCHAR DeviceName[MAX_PATH];
	PVOID Body;
	ULONG BodyLen;
} 
PIPE_MESSAGE, *PPIPE_MESSAGE;

extern NTKERNELAPI HANDLE PsGetCurrentThreadId();
extern NTKERNELAPI HANDLE PsGetProcessId(PEPROCESS Process);

NTSTATUS GetDataFromIrp( IN PIRP Irp, IN PIO_STACK_LOCATION Stack, IN PVOID *InputBuffer, OUT PULONG InputBufferLength );
NTSTATUS HandleInterceptedIrp( IN PHOOKED_DRIVER Driver, IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp );

VOID FreePipeMessage( IN PSNIFFED_DATA pMessage );