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
	PVOID InputBuffer;
	ULONG InputBufferLen;
    ULONG OutputBufferLen;
} 
PIPE_MESSAGE, *PPIPE_MESSAGE;

extern NTKERNELAPI HANDLE PsGetCurrentThreadId();
extern NTKERNELAPI HANDLE PsGetProcessId(PEPROCESS Process);

NTSTATUS HandleInterceptedIrp(IN PHOOKED_DRIVER Driver, IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp);

/*
NTSTATUS ExtractDeviceIoctlIrpData( IN PIRP Irp, OUT PVOID *InputBuffer, OUT PULONG InputBufferLength );
NTSTATUS ExtractReadIrpData(IN PIRP Irp, OUT PVOID *InputBuffer, OUT PULONG InputBufferLength);
NTSTATUS ExtractWriteIrpData(IN PIRP Irp, OUT PVOID *InputBuffer, OUT PULONG InputBufferLength);
*/

VOID FreePipeMessage( IN PSNIFFED_DATA pMessage );