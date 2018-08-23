#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"
#include "PipeComm.h"

#define CFB_QUEUE_SIZE 1024


NTSTATUS InitializeQueue();
NTSTATUS FreeQueue();
NTSTATUS FlushQueue();
UINT32 GetQueueNextFreeSlotIndex();
PVOID PopFromQueue();
NTSTATUS PushToQueue( PVOID pData, PUINT32 lpdwIndex );
PVOID GetItemInQueue( UINT32 Index );