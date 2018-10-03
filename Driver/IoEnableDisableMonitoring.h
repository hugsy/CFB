#pragma once

#include "Common.h"
#include "Driver.h"

NTSTATUS HandleIoEnableMonitoring( PIRP Irp, PIO_STACK_LOCATION Stack );
NTSTATUS HandleIoDisableMonitoring( PIRP Irp, PIO_STACK_LOCATION Stack );