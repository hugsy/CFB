#include "IoSetEventPointer.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HandleIoSetEventPointer)
#endif

#pragma auto_inline(off)


/*++

This function sets the event pointer that is used to notify of new activity (i.e new message pushed 
to the queue). It reads from the IRP the handle from usermode, and performs the adequate checks.

--*/
NTSTATUS HandleIoSetEventPointer( PIRP Irp, PIO_STACK_LOCATION Stack ) 
{
	NTSTATUS status = STATUS_SUCCESS;

	UINT32 dwInputLength = Stack->Parameters.DeviceIoControl.InputBufferLength;
	if (dwInputLength < sizeof(HANDLE) )
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	PHANDLE pHandle = (PHANDLE)Irp->AssociatedIrp.SystemBuffer;
	HANDLE hEvent = *pHandle;
	PKEVENT pKernelNotifEvent;

	CfbDbgPrintInfo( L"HandleIoSetEventPointer() - look up for handle %x\n", hEvent );

	status = ObReferenceObjectByHandle(
		hEvent,
		EVENT_ALL_ACCESS,
		*ExEventObjectType,
		KernelMode,
		&pKernelNotifEvent,
		NULL
	);

	if ( !NT_SUCCESS( status ) )
	{
		return status;
	}

	PKEVENT pOldEvent = InterlockedExchangePointer((PVOID*)&g_EventNotificationPointer, pKernelNotifEvent );

	if ( pOldEvent != NULL )
	{
		ObDereferenceObject( pOldEvent );
	}
	
	return status;
}

#pragma auto_inline()