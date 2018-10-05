#include "Queue.h"


static PVOID* g_CfbQueue = NULL;


/*++

Initialize the structure of the queue.

--*/
NTSTATUS InitializeQueue() 
{
	g_CfbQueue = (PVOID*)ExAllocatePoolWithTag( NonPagedPool, CFB_QUEUE_SIZE*sizeof( PVOID ), CFB_DEVICE_TAG );
	
	if ( !g_CfbQueue )
	{
		CfbDbgPrintErr( L"Failed to allocate queue: insufficient memory in the free pool\n" );
		return STATUS_UNSUCCESSFUL;
	}

	RtlSecureZeroMemory( g_CfbQueue, CFB_QUEUE_SIZE * sizeof( PVOID ) );

	CfbDbgPrintOk( L"Message queue initialized at %p\n", g_CfbQueue );
	return STATUS_SUCCESS;
}


/*++

Free the structure of the queue.

--*/
NTSTATUS FreeQueue()
{
	ExFreePoolWithTag( g_CfbQueue, CFB_DEVICE_TAG );
	CfbDbgPrintOk( L"Message queue %p freeed\n", g_CfbQueue );
	g_CfbQueue = NULL;
	return STATUS_SUCCESS;
}


/*++

--*/
static inline UINT32 GetQueueNextFreeSlotIndex()
{
	UINT32 dwResult = 0;

	while ( g_CfbQueue[dwResult] != NULL )
	{
		if ( dwResult == CFB_QUEUE_SIZE-1 )
			return (UINT32)-1;

		dwResult++;
	}

	return dwResult;
}


/*++

Push a new item at the end of the queue.
On success, `lpdwIndex` holds a pointer to the index the newly pushed item.

--*/
NTSTATUS PushToQueue(IN PVOID pData, OUT PUINT32 lpdwIndex)
{
	NTSTATUS Status;

	KeEnterCriticalRegion();

	UINT32 dwIndex = GetQueueNextFreeSlotIndex();
	if ( dwIndex == (UINT32)-1 )
	{
		CfbDbgPrintErr( L"The queue is full, discarding the message...\n" );
		Status = STATUS_INSUFFICIENT_RESOURCES;
	} 
	else
	{
		g_CfbQueue[dwIndex]=pData;
		*lpdwIndex = dwIndex;
		Status = STATUS_SUCCESS;
	}
	
	KeLeaveCriticalRegion();

	return Status;
}


/*++

Pops the first element out of the queue.

--*/
PVOID PopFromQueue()
{
	PVOID pData = NULL;

	KeEnterCriticalRegion();

	UINT32 dwIndex = GetQueueNextFreeSlotIndex();

	if ( dwIndex > 0 )
	{
		pData = (PVOID)g_CfbQueue[0];

		for ( UINT32 i=0; i < dwIndex; i++ )
		{
			g_CfbQueue[i] = g_CfbQueue[i + 1];
		}

		g_CfbQueue[dwIndex] = NULL;
	}

	KeLeaveCriticalRegion();

	return pData;
}


/*++

Empty the entire queue.

--*/
NTSTATUS FlushQueue()
{
	UINT32 dwIndex = GetQueueNextFreeSlotIndex();

	if ( dwIndex == (UINT32)-1)
	{
		return STATUS_BUFFER_OVERFLOW;
	}

	if ( dwIndex == 0 )
	{
		return STATUS_SUCCESS;
	}

	CfbDbgPrintInfo( L"In FlushQueue()...\n");

	for ( UINT32 i=0; i < dwIndex; i++ )
	{
		FreePipeMessage( g_CfbQueue[i] );
		g_CfbQueue[i] = NULL;
	}


	return STATUS_SUCCESS;
}


/*++

Get a pointer to the `Index` element of the queue, or NULL if the index doesn't exist.

--*/
PVOID GetItemInQueue( IN UINT32 Index )
{
	UINT32 dwIndex = GetQueueNextFreeSlotIndex();

	if ( dwIndex == (UINT32)-1 || Index >= dwIndex )
	{
		return NULL;
	}

	return g_CfbQueue[Index];
}