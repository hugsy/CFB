#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"
#include "Queue.h"
#include "HookedDrivers.h"
#include "IoSetEventPointer.h"



extern NTKERNELAPI HANDLE PsGetCurrentThreadId();
extern NTKERNELAPI HANDLE PsGetProcessId(PEPROCESS Process);


NTSTATUS HandleInterceptedIrp(IN PHOOKED_DRIVER Driver, IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp, OUT PINTERCEPTED_IRP* pIrpOut);
NTSTATUS CompleteHandleInterceptedIrp(_In_ PIRP Irp, _In_ NTSTATUS IrpStatus, _Inout_ PINTERCEPTED_IRP pIrpInfo);

VOID FreeInterceptedIrp( IN PINTERCEPTED_IRP pMessage );