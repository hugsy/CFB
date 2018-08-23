#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"
#include "Queue.h"
#include "HookedDrivers.h"


extern NTKERNELAPI HANDLE PsGetCurrentThreadId();
extern NTKERNELAPI HANDLE PsGetProcessId(PEPROCESS Process);


NTSTATUS GetDataFromIrp(IN PIRP Irp, IN PIO_STACK_LOCATION Stack, IN PVOID *Buffer);
NTSTATUS HandleInterceptedIrp( IN PHOOKED_DRIVER Driver, IN PIRP Irp, IN PIO_STACK_LOCATION Stack );

VOID FreePipeMessage( IN PSNIFFED_DATA pMessage );