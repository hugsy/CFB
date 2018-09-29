#pragma once

#include "Common.h"
#include "Utils.h"

PKEVENT g_EventNotificationPointer;

NTSTATUS HandleIoSetEventPointer( PIRP Irp, PIO_STACK_LOCATION Stack );
