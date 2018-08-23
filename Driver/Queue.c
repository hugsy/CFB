#include "Queue.h"


static PVOID* g_CfbQueue = NULL;


/*++

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

--*/
NTSTATUS FreeQueue()
{
	ExFreePoolWithTag( g_CfbQueue, CFB_DEVICE_TAG );
	CfbDbgPrintOk( L"Message queue %p freeed\n", g_CfbQueue );
	return STATUS_SUCCESS;
}


/*++

--*/
UINT32 GetQueueNextFreeSlotIndex()
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

--*/
NTSTATUS PushToQueue(PVOID pData, PUINT32 lpdwIndex)
{
	// todo: this could be raced, protect section
	UINT32 dwIndex = GetQueueNextFreeSlotIndex();

	if ( dwIndex == (UINT32)-1 )
	{
		CfbDbgPrintErr( L"The queue is full, discarding the message...\n" );
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	g_CfbQueue[dwIndex] = pData;
	*lpdwIndex = dwIndex;

	CfbDbgPrintOk( L"PushToQueue(%p, %d) ok...\n", g_CfbQueue, dwIndex );
	return STATUS_SUCCESS;
}

/*++

--*/
PVOID PopFromQueue()
{
	// todo: this could be raced, protect section
	UINT32 dwIndex = GetQueueNextFreeSlotIndex();

	if ( dwIndex == (UINT32)-1 || dwIndex == 0 )
	{
		// empty list
		CfbDbgPrintErr( L"The queue is empty, cannot pop...\n" );
		return NULL;
	}

	PVOID pData = (PVOID)g_CfbQueue[0];

	// reorder the pointers
	for ( UINT32 i=0; i < dwIndex; i++ )
	{
		g_CfbQueue[i] = g_CfbQueue[i + 1];
	}

	g_CfbQueue[dwIndex] = NULL;

	CfbDbgPrintOk( L"PopFromQueue() = %p ok...\n", pData );

	return pData;
}


/*++

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

--*/
PVOID GetItemInQueue( UINT32 Index )
{
	UINT32 dwIndex = GetQueueNextFreeSlotIndex();

	if ( dwIndex == (UINT32)-1 || Index >= dwIndex )
	{
		return NULL;
	}

	return g_CfbQueue[Index];
}