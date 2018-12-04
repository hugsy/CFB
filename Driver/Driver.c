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
NTSTATUS DriverReadRoutine( PDEVICE_OBJECT pDeviceObject, PIRP Irp )
{
	UNREFERENCED_PARAMETER( pDeviceObject );

	PAGED_CODE();

	NTSTATUS Status;

	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation( Irp );

	if ( !pStack )
	{
		CfbDbgPrintErr( L"IoGetCurrentIrpStackLocation() failed (IRP %p)\n", Irp );
		CompleteRequest( Irp, STATUS_UNSUCCESSFUL, 0 );
		return STATUS_UNSUCCESSFUL;
	}


	ULONG BufferSize = pStack->Parameters.Read.Length;

	PINTERCEPTED_IRP pInterceptedIrp;
	UINT32 dwExpectedSize;

	Status = PeekHeadEntryExpectedSize(&dwExpectedSize);


	if (!NT_SUCCESS(Status))
	{
		if (Status == STATUS_NO_MORE_ENTRIES)
		{
            //
            // Second chance
            //
            KeWaitForSingleObject(g_EventNotificationPointer, UserRequest, KernelMode, FALSE, NULL);
            Status = PeekHeadEntryExpectedSize(&dwExpectedSize);
            if (!NT_SUCCESS(Status))
            {
                CompleteRequest(Irp, Status, 0);
                return Status;
            }
		}
        else
        {
            CompleteRequest(Irp, Status, 0);
            return Status;
        }
	}


	if ( BufferSize == 0 )
	{
		//
		// If BufferSize == 0, the client is probing for the size of the IRP raw data to allocate
		// We end the IRP and announce the expected size
		//
		CompleteRequest( Irp, STATUS_SUCCESS, dwExpectedSize );
		return STATUS_SUCCESS;
	}


	if ( BufferSize != dwExpectedSize )
	{
		CfbDbgPrintErr( L"Buffer is too small, expected %dB, got %dB\n", dwExpectedSize, BufferSize );
		Status = STATUS_INFO_LENGTH_MISMATCH;
		CompleteRequest(Irp, Status, 0);
		return Status;
	}


	PVOID Buffer = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, HighPagePriority );

	if ( !Buffer )
	{
		Status = STATUS_INSUFFICIENT_RESOURCES;
		CompleteRequest(Irp, Status, 0);
		return Status;
	}


	Status = PopFromQueue(&pInterceptedIrp);

	if ( !NT_SUCCESS(Status) )
	{
		Status = STATUS_INSUFFICIENT_RESOURCES;
		CompleteRequest(Irp, Status, 0);
		return Status;
	}


	//
	// Copy header
	//
	PINTERCEPTED_IRP_HEADER pInterceptedIrpHeader = pInterceptedIrp->Header;
	RtlCopyMemory( Buffer, pInterceptedIrpHeader, sizeof(INTERCEPTED_IRP_HEADER) );
	

	//
	// Copy the IRP input buffer (if any)
	//
	if(pInterceptedIrpHeader->InputBufferLength && pInterceptedIrp->RawBuffer)
	{
        PVOID RawBuffer = (PVOID) ( (ULONG_PTR) (Buffer) + sizeof(INTERCEPTED_IRP_HEADER) );
		RtlCopyMemory(RawBuffer, pInterceptedIrp->RawBuffer, pInterceptedIrpHeader->InputBufferLength);
	}

	FreeInterceptedIrp(pInterceptedIrp);

	CompleteRequest( Irp, STATUS_SUCCESS, dwExpectedSize );

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
	CfbDbgPrintInfo(L"Major = 0x%x\n", IoGetCurrentIrpStackLocation(Irp)->MajorFunction);
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
        ReleaseLastTestCase();
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


	RtlInitUnicodeString(&name, CFB_DEVICE_NAME);
	RtlInitUnicodeString(&symLink, CFB_DEVICE_LINK);


	//
	// Create the device
	//

	status = IoCreateDevice(
		DriverObject,
		0,
		&name,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		TRUE,
		&DeviceObject
	);

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

	DeviceObject->Flags |= DO_DIRECT_IO; 

	DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    
    //
    // Initializing the locks and structures
    //
	IoInitializeRemoveLock(&DriverRemoveLock, CFB_DEVICE_TAG, 0, 0);
	InitializeListHead(g_HookedDriversHead);
    KeInitializeSpinLock(&SpinLockOwner);
    InitializeMonitoringStructures();
    InitializeHookedDriverStructures();
	InitializeQueueStructures();
    InitializeTestCaseStructures();
	
	return status;
}



/*++

This routine is the CFB interception routine that will be executed *before* the original
routine from the hooked driver.

Links:
 - https://msdn.microsoft.com/en-us/library/windows/hardware/ff550694(v=vs.85).aspx

--*/
typedef NTSTATUS(*PDRIVER_DISPATCH)(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS InterceptGenericRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
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
        PLIST_ENTRY Entry = g_HookedDriversHead->Flink;

	    do 
	    {
            curDriver = CONTAINING_RECORD(Entry, HOOKED_DRIVER, ListEntry);

		    if (curDriver->DriverObject == DeviceObject->DriverObject)
		    {
			    Found = TRUE;
			    break;
		    }

            Entry = Entry->Flink;

        } 
        while (Entry != g_HookedDriversHead);
    }


	if (!Found)
	{
		//
		// This is really bad: it means our interception routine got called by a non-hooked driver
		// Could be a bad pointer restoration. Anyway, we log and fail for now.
		//

		CfbDbgPrintErr(L"Failed to find a DriverObject associated to the received IRP\n");
		Status = CompleteRequest( Irp, STATUS_NO_SUCH_DEVICE, 0 );

	} 
	else
	{

		//
		// Capture the IRP data
		//

		if ( IsMonitoringEnabled() && curDriver->Enabled == TRUE )
		{
			Status = HandleInterceptedIrp( curDriver, DeviceObject, Irp );

			if( !NT_SUCCESS(Status) && Status != STATUS_NOT_IMPLEMENTED)
            {
				CfbDbgPrintWarn( L"HandleInterceptedIrp() failed (status=0x%X)\n", Status );
            }
		}


		//
		// Call the original routine
		//

        DWORD dwIndex = Stack->MajorFunction;
        PDRIVER_DISPATCH OldRoutine = (DRIVER_DISPATCH*)curDriver->OriginalRoutines[dwIndex];
        Status = OldRoutine(DeviceObject, Irp);

        /*
		switch ( Stack->MajorFunction )
		{
		case IRP_MJ_READ:
			OldRoutine = (DRIVER_DISPATCH*)curDriver->OldReadRoutine;
			Status = OldRoutine( DeviceObject, Irp );
			break;

		case IRP_MJ_WRITE:
			OldRoutine = (DRIVER_DISPATCH*)curDriver->OldWriteRoutine;
			Status = OldRoutine( DeviceObject, Irp );
			break;

		case IRP_MJ_DEVICE_CONTROL:
			OldRoutine = (DRIVER_DISPATCH*)curDriver->OldDeviceControlRoutine;
			Status = OldRoutine( DeviceObject, Irp );
			break;

		default:
			CfbDbgPrintErr( L"Fail to fallback to the IRP MAJOR routine %x in '%s'\n", Stack->MajorFunction, curDriver->Name );
			Status = STATUS_NOT_IMPLEMENTED;
			CompleteRequest( Irp, STATUS_NOT_IMPLEMENTED, 0 );
			break;
		}
        */
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

    case IOCTL_StoreTestCase:
        CfbDbgPrintInfo(L"Received 'IoctlStoreTestCase'\n");
        Status = HandleIoStoreTestCase(Irp, CurrentStack);
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

