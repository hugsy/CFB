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

	return STATUS_SUCCESS;
}


/*++

--*/
NTSTATUS FreeQueue()
{
	ExFreePoolWithTag( g_CfbQueue, CFB_DEVICE_TAG );

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
NTSTATUS PushToQueue( PVOID pData)
{
	UINT32 dwIndex = GetQueueNextFreeSlotIndex();

	if ( dwIndex == (UINT32)-1 )
	{
		CfbDbgPrintErr( L"The queue is full, discarding the message...\n" );
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	g_CfbQueue[dwIndex] = pData;

	return STATUS_SUCCESS;
}

/*++

--*/
PVOID PopFromQueue()
{
	// TODO critical section here
	UINT32 dwIndex = GetQueueNextFreeSlotIndex();

	if ( dwIndex == (UINT32)-1 || dwIndex == 0 )
	{
		// empty list
		return NULL;
	}

	PVOID pData = (PVOID)g_CfbQueue[0];

	// reorder the pointers
	for ( UINT32 i=0; i < dwIndex; i++ )
	{
		g_CfbQueue[i] = g_CfbQueue[i + 1];
	}

	g_CfbQueue[dwIndex] = NULL;

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


	for ( UINT32 i=0; i < dwIndex; i++ )
	{
		FreePipeMessage( g_CfbQueue[i] );
		g_CfbQueue[i] = NULL;
	}


	return STATUS_SUCCESS;
}