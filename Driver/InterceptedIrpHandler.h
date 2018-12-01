#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"
#include "Queue.h"
#include "HookedDrivers.h"
#include "IoSetEventPointer.h"



extern NTKERNELAPI HANDLE PsGetCurrentThreadId();
extern NTKERNELAPI HANDLE PsGetProcessId(PEPROCESS Process);

NTSTATUS HandleInterceptedIrp(IN PHOOKED_DRIVER Driver, IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp);

VOID FreeInterceptedIrp( IN PINTERCEPTED_IRP pMessage );