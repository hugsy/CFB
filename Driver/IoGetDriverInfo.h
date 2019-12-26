#pragma once

#include "Common.h"
#include "Driver.h"
#include "HookedDrivers.h"



NTSTATUS HandleIoGetDriverInfo(_In_ PIRP Irp, _Inout_ PIO_STACK_LOCATION Stack, _Out_ PULONG pdwDataWritten);
NTSTATUS HandleIoGetNumberOfHookedDrivers(_In_ PIRP Irp, _Inout_ PIO_STACK_LOCATION Stack, _Out_ PULONG pdwDataWritten);
NTSTATUS HandleIoGetNamesOfHookedDrivers(_In_ PIRP Irp, _Inout_ PIO_STACK_LOCATION Stack, _Out_ PULONG pdwDataWritten);