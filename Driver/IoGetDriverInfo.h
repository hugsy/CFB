#pragma once

#include "Common.h"
#include "Driver.h"
#include "HookedDrivers.h"


NTSTATUS HandleIoGetDriverInfo(PIRP Irp, PIO_STACK_LOCATION Stack);
NTSTATUS HandleIoGetNumberOfHookedDrivers(PIRP Irp, PIO_STACK_LOCATION Stack);

