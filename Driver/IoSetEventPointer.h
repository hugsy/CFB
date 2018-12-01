#pragma once

#include "Common.h"
#include "Utils.h"

PKEVENT g_EventNotificationPointer;

NTSTATUS HandleIoSetEventPointer( IN PIRP Irp, IN PIO_STACK_LOCATION Stack );
VOID ClearNotificationPointer();
VOID SetNewIrpInQueueAlert();
VOID UnsetNewIrpInQueueAlert();