#ifndef __IO_ADD_DRIVER_H__
#define __IO_ADD_DRIVER_H__

#pragma once

#include "Common.h"
#include "Driver.h"
#include "HookedDrivers.h"

#include <Ntstrsafe.h>


NTSTATUS HandleIoAddDriver(PIRP Irp, PIO_STACK_LOCATION Stack);

#endif