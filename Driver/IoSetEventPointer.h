#pragma once

#include "Common.h"
#include "Utils.h"



NTSTATUS HandleIoSetEventPointer( IN PIRP Irp, IN PIO_STACK_LOCATION Stack );
VOID ClearNotificationPointer();
VOID NotifyClient();