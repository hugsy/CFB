#pragma once

#include "Common.h"
#include "Driver.h"


NTSTATUS HandleIoEnableDriverMonitoring(_In_ PIRP Irp, _In_ PIO_STACK_LOCATION Stack);
NTSTATUS HandleIoDisableDriverMonitoring(_In_ PIRP Irp, _In_ PIO_STACK_LOCATION Stack);