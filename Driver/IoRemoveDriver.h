#pragma once

#include "Common.h"
#include "Driver.h"
#include "HookedDrivers.h"

#include "IoAddDriver.h"

NTSTATUS HandleIoRemoveDriver(PIRP Irp, PIO_STACK_LOCATION Stack);
NTSTATUS RemoveAllDrivers();

