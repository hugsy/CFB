#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"
#include "Queue.h"
#include "HookedDrivers.h"
#include "IoSetEventPointer.h"


#define CFB_FASTIO_USE_OUTPUT_BUFFER 0
#define CFB_FASTIO_USE_INPUT_BUFFER 1
#define CFB_FASTIO_INIT_QUEUE_MESSAGE 2


extern NTKERNELAPI HANDLE PsGetCurrentThreadId();
extern NTKERNELAPI HANDLE PsGetProcessId(PEPROCESS Process);


/**
 *
 * IOCTL hooked IRP handling
 *
 */
NTSTATUS HandleInterceptedIrp(IN PHOOKED_DRIVER Driver, IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp, OUT PINTERCEPTED_IRP* pIrpOut);
NTSTATUS CompleteHandleInterceptedIrp(_In_ PIO_STACK_LOCATION Stack, _In_ PVOID UserBuffer, _In_ NTSTATUS IrpStatus, _Inout_ PINTERCEPTED_IRP pIrpInfo);


/**
 *
 * IOCTL hooked IRP handling
 *
 */
NTSTATUS
HandleInterceptedFastIo(
	_In_ PHOOKED_DRIVER Driver,
	_In_ PDEVICE_OBJECT pDeviceObject,
	_In_ UINT32 Type,
	_In_ UINT32 IoctlCode,
	_In_ PVOID InputBuffer,
	_In_ ULONG InputBufferLength,
	_In_ UINT32 Flags,
	_Inout_ PINTERCEPTED_IRP* pIrpOut
);


/**
 *
 * Intercepted IRP cleanup 
 *
 */
VOID FreeInterceptedIrp( IN PINTERCEPTED_IRP pMessage );