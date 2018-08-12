#ifndef __IO_GET_DRIVER_INFO_H__
#define __IO_GET_DRIVER_INFO_H__

#pragma once

#include "Common.h"
#include "Driver.h"
#include "HookedDrivers.h"


NTSTATUS HandleIoGetDriverInfo(PIRP Irp, PIO_STACK_LOCATION Stack);
NTSTATUS HandleIoGetNumberOfHookedDrivers(PIRP Irp, PIO_STACK_LOCATION Stack);

#endif /* __IO_GET_DRIVER_INFO_H__ */