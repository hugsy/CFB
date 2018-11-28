#include "Driver.h"


#ifdef ALLOC_PRAGMA

//
// Discard after initialization
//
#pragma alloc_text (INIT, DriverEntry)

#endif



static IO_REMOVE_LOCK DriverRemoveLock;
static KSPIN_LOCK g_SpinLock;
static KLOCK_QUEUE_HANDLE g_SpinLockQueue;

//
// Not more than one process can interact with the device object
//
static PEPROCESS pCurrentOwnerProcess;
static KSPIN_LOCK SpinLockOwner;
static KLOCK_QUEUE_HANDLE SpinLockQueueOwner;

extern PLIST_ENTRY g_HookedDriversHead;


/*++

--*/
NTSTATUS DriverReadRoutine( PDEVICE_OBJECT pDeviceObject, PIRP pIrp )
{
	UNREFERENCED_PARAMETER( pDeviceObject );

	PAGED_CODE();

	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation( pIrp );

	if ( !pStack )
	{
		CfbDbgPrintErr( L"IoGetCurrentIrpStackLocation() failed (IRP %p)\n", pIrp );
		CompleteRequest( pIrp, STATUS_UNSUCCESSFUL, 0 );
		return STATUS_UNSUCCESSFUL;
	}


	ULONG BufferSize = pStack->Parameters.Read.Length;

	PSNIFFED_DATA pData = (PSNIFFED_DATA)GetItemInQueue(0);
	if ( !pData )
	{
		CompleteRequest( pIrp, STATUS_SUCCESS, 0 );
		return STATUS_SUCCESS;
	}


	UINT32 dwExpectedSize = sizeof( SNIFFED_DATA_HEADER ) + pData->Header->InputBufferLength;

	if ( BufferSize == 0 )
	{
		CompleteRequest( pIrp, STATUS_SUCCESS, dwExpectedSize );
		return STATUS_SUCCESS;
	}

	if ( BufferSize < dwExpectedSize )
	{
		CfbDbgPrintErr( L"Buffer is too small, expected %dB, got %dB\n", dwExpectedSize, BufferSize );
		CompleteRequest( pIrp, STATUS_BUFFER_TOO_SMALL, 0 );
		return STATUS_BUFFER_TOO_SMALL;
	}


	PVOID Buffer = MmGetSystemAddressForMdlSafe( pIrp->MdlAddress, HighPagePriority );

	if ( !Buffer )
	{
		CfbDbgPrintErr( L"Input buffer is NULL\n" );
		return STATUS_INSUFFICIENT_RESOURCES;
	}


	pData = PopFromQueue();
	if ( !pData )
	{
		return STATUS_SUCCESS;
	}

	CfbDbgPrintOk(L"Poped message %p\n", pData);


	//
	// Copy header
	//
	PSNIFFED_DATA_HEADER pHeader = pData->Header;
	RtlCopyMemory( Buffer, pHeader, sizeof( SNIFFED_DATA_HEADER ) );
	
	//
	// Copy the IRP input buffer (if any)
	//
	if( pHeader->InputBufferLength && pData->InputBuffer)
	{
        PVOID InputBuffer = (PVOID) ( (ULONG_PTR) (Buffer) + sizeof(SNIFFED_DATA_HEADER) );
		RtlCopyMemory(InputBuffer, pData->InputBuffer, pHeader->InputBufferLength);
	}

	FreePipeMessage( pData );

	CompleteRequest( pIrp, STATUS_SUCCESS, dwExpectedSize );

	return STATUS_SUCCESS;
}


/*++

Generic routine for unsupported major types.

--*/
NTSTATUS IrpNotImplementedHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PAGED_CODE();

#ifdef _DEBUG
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation( Irp );
	CfbDbgPrintInfo(L"Major = 0x%x\n", Stack->MajorFunction);
#endif

	CompleteRequest(Irp, STATUS_NOT_IMPLEMENTED, 0);
	return STATUS_NOT_IMPLEMENTED;
}


/*++

--*/
NTSTATUS DriverCleanup( PDEVICE_OBJECT DeviceObject, PIRP Irp )
{
	UNREFERENCED_PARAMETER( DeviceObject );

	PAGED_CODE();

	   
	//
	// Unswap all hooked drivers left to avoid new IRPs to be handled by us
	//

	KeEnterCriticalRegion();

    {
        DisableMonitoring();
        RemoveAllDrivers();
        IoAcquireRemoveLock(&DriverRemoveLock, Irp);
        IoReleaseRemoveLockAndWait(&DriverRemoveLock, Irp);
    }

    KeLeaveCriticalRegion();

	FlushQueue();


	//
	// Disable events and clear the queue
	//

	ClearNotificationPointer();

	FreeQueue();

	CompleteRequest( Irp, STATUS_SUCCESS, 0 );

	return STATUS_SUCCESS;
}


/*++

Driver entry point: create the driver object for CFB

--*/
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	PAGED_CODE();

	CfbDbgPrintInfo(L"Loading driver IrpDumper\n");

	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PDEVICE_OBJECT DeviceObject;
	UNICODE_STRING name, symLink;

	if ( !NT_SUCCESS( InitializeQueue() ) )
	{
		return STATUS_UNSUCCESSFUL;
	}

    InitializeListHead(g_HookedDriversHead);

	RtlInitUnicodeString(&name, CFB_DEVICE_NAME);
	RtlInitUnicodeString(&symLink, CFB_DEVICE_LINK);


	//
	// Create the device
	//

	status = IoCreateDevice(DriverObject,
							0,
							&name,
							FILE_DEVICE_UNKNOWN,
							FILE_DEVICE_SECURE_OPEN,
							TRUE,
							&DeviceObject);

	if( !NT_SUCCESS(status) )
	{
		CfbDbgPrintErr(L"Error creating device object (0x%08X)\n", status);
		if (DeviceObject)
		{
			IoDeleteDevice(DeviceObject);
		}

		return status;
	}

	CfbDbgPrintOk( L"Device object '%s' created\n", CFB_DEVICE_NAME );


	//
	// Populate the IRP handlers
	//

	for (DWORD i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = IrpNotImplementedHandler;
	}

	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCloseRoutine;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateRoutine;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDeviceControlRoutine;
	DriverObject->MajorFunction[IRP_MJ_READ] = DriverReadRoutine;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DriverCleanup;
	DriverObject->DriverUnload = DriverUnloadRoutine;
	
	CfbDbgPrintOk( L"Driver object '%s' routines populated\n", CFB_DEVICE_NAME );


	//
	// Create the symlink
	//
	status = IoCreateSymbolicLink(&symLink, &name);
	if (!NT_SUCCESS(status))
	{
		CfbDbgPrintErr(L"Error creating symbolic link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);
	}
	else
	{
		CfbDbgPrintOk( L"Symlink '%s' to driver object '%s' created\n", CFB_DEVICE_LINK, CFB_DEVICE_NAME );
	}

	DeviceObject->Flags |= DO_DIRECT_IO; // TODO: use DO_BUFFERED_IO instead ?

	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    
    //
    // Initializing the locks and structures
    //
    KeInitializeSpinLock(&SpinLockOwner);
    InitializeMonitoringStructures();
    InitializeHookedDriverStructures();

	IoInitializeRemoveLock( &DriverRemoveLock, CFB_DEVICE_TAG, 0, 0 );

	return status;
}



/*++

This routine is the CFB interception routine that will be executed *before* the original
routine from the hooked driver.

Links:
 - https://msdn.microsoft.com/en-us/library/windows/hardware/ff550694(v=vs.85).aspx

--*/
typedef NTSTATUS(*PDRIVER_DISPATCH)(PDEVICE_OBJECT DeviceObject, PIRP Irp);

static NTSTATUS InterceptGenericRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	NTSTATUS Status;
    BOOLEAN Found = FALSE;
    PHOOKED_DRIVER curDriver = NULL;

	Status = IoAcquireRemoveLock( &DriverRemoveLock, Irp );
	if ( !NT_SUCCESS( Status ) )
	{
		return Status;
	}

    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);

	//
	// Find the original function for the driver
	//

    if( !IsListEmpty(g_HookedDriversHead) )
    {
	    
        PLIST_ENTRY Entry = g_HookedDriversHead;

	    do 
	    {
            curDriver = CONTAINING_RECORD(Entry, HOOKED_DRIVER, ListEntry);

		    if (curDriver->DriverObject == DeviceObject->DriverObject)
		    {
			    Found = TRUE;
			    break;
		    }

            Entry = Entry->Flink;
        } while (Entry != g_HookedDriversHead);
    }


	if (!Found)
	{
		//
		// This is really bad: it means our interception routine got called by a non-hooked driver
		// Could be a bad pointer restoration. Anyway, we log and fail for now.
		//

		CfbDbgPrintErr(L"InterceptedGenericRoutine() failed: couldn't get the current DriverObject\n");
		Status = CompleteRequest( Irp, STATUS_NO_SUCH_DEVICE, 0 );

	} 
	else
	{

		//
		// Push the message to the named pipe
		//

		if ( IsMonitoringEnabled() && curDriver->Enabled == TRUE )
		{
			Status = HandleInterceptedIrp( curDriver, DeviceObject, Irp );
			if( !NT_SUCCESS(Status) )
				CfbDbgPrintWarn( L"HandleInterceptedIrp() returned 0x%X\n", Status );
		}


		//
		// Call the original routine
		//

		PDRIVER_DISPATCH OldRoutine;

		switch ( Stack->MajorFunction )
		{
		case IRP_MJ_READ:
			//CfbDbgPrintInfo( L"Fallback to IRP_MJ_READ routine at %p for '%s'\n", curDriver->OldReadRoutine, curDriver->Name );
			OldRoutine = (DRIVER_DISPATCH*)curDriver->OldReadRoutine;
			Status = OldRoutine( DeviceObject, Irp );
			break;

		case IRP_MJ_WRITE:
			//CfbDbgPrintInfo( L"Fallback to IRP_MJ_WRITE routine at %p for '%s'\n", curDriver->OldWriteRoutine, curDriver->Name );
			OldRoutine = (DRIVER_DISPATCH*)curDriver->OldWriteRoutine;
			Status = OldRoutine( DeviceObject, Irp );
			break;

		case IRP_MJ_DEVICE_CONTROL:
			//CfbDbgPrintInfo( L"Fallback to IRP_MJ_DEVICE_CONTROL routine at %p for '%s'\n", curDriver->OldDeviceControlRoutine, curDriver->Name );
			OldRoutine = (DRIVER_DISPATCH*)curDriver->OldDeviceControlRoutine;
			Status = OldRoutine( DeviceObject, Irp );
			break;

		default:
			CfbDbgPrintErr( L"Fail to fallback to the IRP MAJOR routine %x in '%s'\n", Stack->MajorFunction, curDriver->Name );
			Status = STATUS_NOT_IMPLEMENTED;
			CompleteRequest( Irp, STATUS_NOT_IMPLEMENTED, 0 );
			break;
		}

	}	

	IoReleaseRemoveLock( &DriverRemoveLock, Irp );

	return Status;
}


/*++

Unload routine for CFB IrpDumper.

--*/
VOID DriverUnloadRoutine(PDRIVER_OBJECT DriverObject)
{  

	//
	// Delete the device object
	//

	UNICODE_STRING symLink = { 0, };
	RtlInitUnicodeString(&symLink, CFB_DEVICE_LINK);

	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);


	CfbDbgPrintOk(L"Success unloading '%s'\n", CFB_PROGRAM_NAME_SHORT);

	return;
}


/*++

Generic function for IRP completion

--*/
NTSTATUS CompleteRequest(PIRP Irp, NTSTATUS status, ULONG_PTR Information)
{
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = Information;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return status;
}


/*++

--*/
NTSTATUS DriverCloseRoutine(PDEVICE_OBJECT pObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pObject);

	PAGED_CODE();

    KeAcquireInStackQueuedSpinLock(&SpinLockOwner, &SpinLockQueueOwner);

    if (pCurrentOwnerProcess != NULL)
    {
        pCurrentOwnerProcess = NULL;
        CfbDbgPrintOk(L"Unlocked device...\n");
    }

    KeReleaseInStackQueuedSpinLock(&SpinLockQueueOwner);

	return CompleteRequest(Irp, STATUS_SUCCESS, 0);
}


/*++

--*/
NTSTATUS DriverCreateRoutine(PDEVICE_OBJECT pObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(pObject);

    PAGED_CODE();

    NTSTATUS Status;

    KeAcquireInStackQueuedSpinLock(&SpinLockOwner, &SpinLockQueueOwner);
    
    if (pCurrentOwnerProcess == NULL)
    {
        pCurrentOwnerProcess = PsGetCurrentProcess();
        CfbDbgPrintOk(L"Locked device to process %p...\n", pCurrentOwnerProcess);
        Status = STATUS_SUCCESS;
    }   
    else
    {
        Status = STATUS_DEVICE_ALREADY_ATTACHED;
    }

    KeReleaseInStackQueuedSpinLock(&SpinLockQueueOwner);

    return CompleteRequest(Irp, Status, 0);
}


/*++

--*/
NTSTATUS DriverDeviceControlRoutine(PDEVICE_OBJECT pObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pObject);

	PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG_PTR Information = 0;


    if ( pCurrentOwnerProcess != PsGetCurrentProcess() )
    {
        Status = STATUS_DEVICE_ALREADY_ATTACHED;
        CompleteRequest(Irp, Status, Information);
        return Status;
    }
	
	PIO_STACK_LOCATION CurrentStack = IoGetCurrentIrpStackLocation(Irp);
	ULONG IoctlCode = CurrentStack->Parameters.DeviceIoControl.IoControlCode;

	switch (IoctlCode)
	{

	case IOCTL_AddDriver:
		CfbDbgPrintInfo(L"Received 'IoctlAddDrivers'\n");
		Status = HandleIoAddDriver(Irp, CurrentStack);
		break;


	case IOCTL_RemoveDriver:
		CfbDbgPrintInfo(L"Received 'IoctlRemoveDriver'\n");
		Status = HandleIoRemoveDriver(Irp, CurrentStack);
		break;


	case IOCTL_EnableMonitoring:
		CfbDbgPrintInfo( L"Received 'IoctlEnableDriver'\n" );
		Status = HandleIoEnableMonitoring( Irp, CurrentStack );
		break;


	case IOCTL_DisableMonitoring:
		CfbDbgPrintInfo( L"Received 'IoctlDisableDriver'\n" );
		Status = HandleIoDisableMonitoring( Irp, CurrentStack );
		break;


	case IOCTL_GetNumberOfDrivers:
		CfbDbgPrintInfo(L"Received 'IoctlGetNumberOfDrivers'\n");
		Status = HandleIoGetNumberOfHookedDrivers(Irp, CurrentStack);
		break;


	case IOCTL_GetDriverInfo:
		CfbDbgPrintInfo(L"Received 'IoctlGetDriverInfo'\n");
		Status = HandleIoGetDriverInfo( Irp, CurrentStack );
		break;


	case IOCTL_SetEventPointer:
		CfbDbgPrintInfo( L"Received 'IoctlSetEventPointer'\n" );
		Status = HandleIoSetEventPointer(Irp, CurrentStack);
		break;


	default:
		CfbDbgPrintErr(L"Received invalid ioctl '%#X'\n", IoctlCode);
		Status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	CfbDbgPrintInfo(L"IOCTL #%x returned %#x\n", IoctlCode, Status);

	CompleteRequest(Irp, Status, Information);

	return Status;
}


/*++

--*/
NTSTATUS InterceptedDeviceControlRoutine( PDEVICE_OBJECT DeviceObject, PIRP Irp )
{
	return InterceptGenericRoutine( DeviceObject, Irp );
}


/*++

--*/
NTSTATUS InterceptedReadRoutine( PDEVICE_OBJECT DeviceObject, PIRP Irp )
{
	return InterceptGenericRoutine( DeviceObject, Irp );
}
	

/*++

--*/
NTSTATUS InterceptedWriteRoutine( PDEVICE_OBJECT DeviceObject, PIRP Irp )
{
	return InterceptGenericRoutine( DeviceObject, Irp );
}

