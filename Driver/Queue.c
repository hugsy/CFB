#include "Queue.h"


//static KSPIN_LOCK IrpQueueSpinLock;
//static KLOCK_QUEUE_HANDLE IrpQueueSpinLockQueue;
//static FAST_MUTEX FlushQueueMutex;
static FAST_MUTEX QueueMutex;
static LIST_ENTRY InterceptedIrpHead;
LIST_ENTRY* g_InterceptedIrpHead = &InterceptedIrpHead;
UINT32 InterceptedIrpListSize;



/*++

Initialize the structure of the queue.

--*/
void InitializeQueueStructures()
{
	//KeInitializeSpinLock(&IrpQueueSpinLock);
	//ExInitializeFastMutex(&FlushQueueMutex);
	ExInitializeFastMutex(&QueueMutex);
	InitializeListHead(g_InterceptedIrpHead); 
	InterceptedIrpListSize = 0;
	return;
}


/*++

Return the current number of IRP data stored in memory.

--*/
UINT32 GetIrpListSize()
{
	return InterceptedIrpListSize;
}


/*++

Push a new item at the end of the queue.

--*/
NTSTATUS PushToQueue(IN PINTERCEPTED_IRP pData)
{
	NTSTATUS Status;

	ExAcquireFastMutex(&QueueMutex);

	if (InterceptedIrpListSize < CFB_QUEUE_SIZE)
	{
		InsertTailList(g_InterceptedIrpHead, &(pData->ListEntry));
		InterceptedIrpListSize++;
		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}

	ExReleaseFastMutex(&QueueMutex);

	return Status;
}


/*++

Peek into the first item of the hooked driver linked list (if any), to determine the 
total size of the intercepted IRP (header + body).

--*/
NTSTATUS PeekHeadEntryExpectedSize(OUT PUINT32 pdwExpectedSize)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
    
	ExAcquireFastMutex(&QueueMutex);

	if (IsListEmpty(g_InterceptedIrpHead))
	{
		*pdwExpectedSize = 0;
		Status = STATUS_NO_MORE_ENTRIES;
	}
	else
	{
		PINTERCEPTED_IRP pFirstIrp = CONTAINING_RECORD(g_InterceptedIrpHead->Flink, INTERCEPTED_IRP, ListEntry);
		*pdwExpectedSize = sizeof(INTERCEPTED_IRP_HEADER) + \
			pFirstIrp->Header->InputBufferLength;
			pFirstIrp->Header->OutputBufferLength;
		Status = STATUS_SUCCESS;
	}

	ExReleaseFastMutex(&QueueMutex);
    
	return Status;
}


/*++

Pops the heading item out of the queue.

--*/
NTSTATUS PopFromQueue(OUT PINTERCEPTED_IRP *pData)
{
	NTSTATUS Status;

	ExAcquireFastMutex(&QueueMutex);

	if (!IsListEmpty(g_InterceptedIrpHead))
	{
		PLIST_ENTRY pListHead = RemoveHeadList(g_InterceptedIrpHead);
		*pData = CONTAINING_RECORD(pListHead, INTERCEPTED_IRP, ListEntry);
		InterceptedIrpListSize--;
		Status = STATUS_SUCCESS;
	}
	else
	{
		*pData = NULL;
		Status = STATUS_NO_MORE_ENTRIES;
	}

	ExReleaseFastMutex(&QueueMutex);

	return Status;
}


/*++

Empty the entire queue.

--*/
NTSTATUS FlushQueue()
{
	NTSTATUS Status = STATUS_SUCCESS; 

	ExAcquireFastMutex(&QueueMutex);

	while (!IsListEmpty(g_InterceptedIrpHead))
	{
		PINTERCEPTED_IRP pIrp;

		Status = PopFromQueue(&pIrp);
		if (!NT_SUCCESS(Status))
		{
			if (Status == STATUS_NO_MORE_ENTRIES)
			{
				Status = STATUS_SUCCESS;
				break;
			}

			CfbDbgPrintErr(L"An error occured : status=0x%x\n", Status);
		}
        else
        {
            FreeInterceptedIrp(pIrp);
        }
	}

	ExReleaseFastMutex(&QueueMutex);
	
	CfbDbgPrintOk( L"Message queue flushed...\n" );

	return STATUS_SUCCESS;
}
