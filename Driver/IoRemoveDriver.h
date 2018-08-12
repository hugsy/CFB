#ifndef __IO_REMOVE_DRIVER_H__
#define __IO_REMOVE_DRIVER_H__

#pragma once

#include "Common.h"
#include "Driver.h"
#include "HookedDrivers.h"

NTSTATUS HandleIoRemoveDriver(PIRP Irp, PIO_STACK_LOCATION Stack);
NTSTATUS RemoveAllDrivers();

#endif /* __IO_REMOVE_DRIVER_H__ */