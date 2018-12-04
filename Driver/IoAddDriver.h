#pragma once

#include "Common.h"
#include "Utils.h"
#include "Driver.h"
#include "HookedDrivers.h"

#include <Ntstrsafe.h>

KSPIN_LOCK g_AddRemoveDriverSpinLock;
KLOCK_QUEUE_HANDLE g_AddRemoveSpinLockQueue;

NTSTATUS HandleIoAddDriver(PIRP Irp, PIO_STACK_LOCATION Stack);
VOID InitializeIoAddDriverStructure();
