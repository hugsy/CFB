#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"
#include "InterceptedIrpHandler.h"


#define CFB_QUEUE_SIZE 1024


void InitializeQueueStructures();

NTSTATUS PushToQueue(IN PINTERCEPTED_IRP pData);
NTSTATUS PopFromQueue(OUT PINTERCEPTED_IRP *pData);
NTSTATUS FlushQueue();
NTSTATUS PeekHeadEntryExpectedSize(OUT PUINT32 pdwExpectedSize);
UINT32 GetIrpListSize();