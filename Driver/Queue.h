#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"
#include "PipeComm.h"

#define CFB_QUEUE_SIZE 1024


NTSTATUS InitializeQueue();
NTSTATUS FreeQueue();
NTSTATUS FlushQueue();
PVOID PopFromQueue();
NTSTATUS PushToQueue( IN PVOID pData, OUT PUINT32 lpdwIndex );
PVOID GetItemInQueue( IN UINT32 Index );