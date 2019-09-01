#include "Driver.h"


#ifdef ALLOC_PRAGMA

//
// Discard after initialization
//
#pragma alloc_text (INIT, DriverEntry)

#endif


static PDEVICE_OBJECT g_DeviceObject;
static IO_REMOVE_LOCK g_DriverRemoveLock;
static KSPIN_LOCK g_SpinLock;
static KLOCK_QUEUE_HANDLE g_SpinLockQueue;

//
// Not more than one process can interact with the device object
//
static PEPROCESS pCurrentOwnerProcess;
static KSPIN_LOCK SpinLockOwner;
static KLOCK_QUEUE_HANDLE SpinLockQueueOwner;

extern PLIST_ENTRY g_HookedDriverHead;


/*++

This routine is called when trying to ReadFile() from a handle to the device IrpDumper.

--*/
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverReadRoutine( PDEVICE_OBJECT pDeviceObject, PIRP Irp )
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
NTSTATUS _Function_class_(DRIVER_DISPATCH) IrpNotImplementedHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp)
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
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverCleanup( PDEVICE_OBJECT DeviceObject, PIRP Irp )
{
	UNREFERENCED_PARAMETER( DeviceObject );

	PAGED_CODE();

	   
	//
	// Unswap all hooked drivers left to avoid new IRPs to be handled by us
	//

	KeEnterCriticalRegion();

    {
		ReleaseTestCaseStructures();
        DisableMonitoring();
        RemoveAllDrivers();
        IoAcquireRemoveLock(&g_DriverRemoveLock, Irp);
        IoReleaseRemoveLockAndWait(&g_DriverRemoveLock, Irp);
    }

    KeLeaveCriticalRegion();

	FlushQueue();


	//
	// Disable events and clear the queue
	//

	ClearNotificationPointer();


	CompleteRequest( Irp, STATUS_SUCCESS, 0 );

	if (g_DeviceObject)
	{
		IoDeleteDevice(g_DeviceObject);
		g_DeviceObject = NULL;
	}
	
	return STATUS_SUCCESS;
}


/*++

Driver entry point: create the driver object for CFB

--*/
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	PAGED_CODE();

	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	CfbDbgPrintInfo(L"Loading driver IrpDumper\n");

	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(CFB_DEVICE_NAME); 
	UNICODE_STRING DeviceSymlink = RTL_CONSTANT_STRING(CFB_DEVICE_LINK);


	//
	// Create the device
	//

	Status = IoCreateDevice(
		DriverObject,
		0,
		&DeviceName,
		FILE_DEVICE_UNKNOWN,
		FILE_DEVICE_SECURE_OPEN,
		TRUE,
		&g_DeviceObject
	);

	if( !NT_SUCCESS(Status) )
	{
		CfbDbgPrintErr(L"Error creating device object (0x%08X)\n", Status);
		return Status;
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
	Status = IoCreateSymbolicLink(&DeviceSymlink, &DeviceName);
	if (!NT_SUCCESS(Status))
	{
		CfbDbgPrintErr(L"Error creating symbolic link (0x%08X)\n", Status);
		IoDeleteDevice(g_DeviceObject);
		return Status;
	}

	CfbDbgPrintOk( L"Symlink '%s' to device object '%s' created\n", CFB_DEVICE_LINK, CFB_DEVICE_NAME );

	g_DeviceObject->Flags |= DO_DIRECT_IO; 
	g_DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    
    //
    // Initializing the locks and structures
    //
	IoInitializeRemoveLock(&g_DriverRemoveLock, CFB_DEVICE_TAG, 0, 0);
	InitializeListHead(g_HookedDriverHead);
    KeInitializeSpinLock(&SpinLockOwner);
    InitializeMonitoringStructures();
    InitializeHookedDriverStructures();
	InitializeQueueStructures();
	InitializeTestCaseStructures();
	InitializeIoAddDriverStructure();
	
	return Status;
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
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
    BOOLEAN Found = FALSE;
    PHOOKED_DRIVER curDriver = NULL;


	Status = IoAcquireRemoveLock( &g_DriverRemoveLock, Irp );
	if ( !NT_SUCCESS( Status ) )
	{
		return Status;
	}

    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);

	//
	// Find the original function for the driver
	//

    if( !IsListEmpty(g_HookedDriverHead) )
    {
        PLIST_ENTRY Entry = g_HookedDriverHead->Flink;

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
        while (Entry != g_HookedDriverHead);
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

		if (IsMonitoringEnabled() && curDriver->Enabled == TRUE && pCurrentOwnerProcess != PsGetCurrentProcess())
		{
			Status = HandleInterceptedIrp(curDriver, DeviceObject, Irp);

			if (!NT_SUCCESS(Status) && Status != STATUS_NOT_IMPLEMENTED)
			{
				CfbDbgPrintWarn(L"HandleInterceptedIrp() failed (status=0x%X)\n", Status);
			}
		}


		//
		// Call the original routine
		//

		DWORD dwIndex = Stack->MajorFunction;
		PDRIVER_DISPATCH OldRoutine = (DRIVER_DISPATCH*)curDriver->OriginalRoutines[dwIndex];
		Status = OldRoutine(DeviceObject, Irp);
	}

	IoReleaseRemoveLock( &g_DriverRemoveLock, Irp );

	return Status;
}


/*++

--*/
BOOLEAN InterceptGenericFastIoDeviceControl(
	PFILE_OBJECT FileObject,
	BOOLEAN Wait,
	IN PVOID InputBuffer OPTIONAL,
	IN ULONG InputBufferLength,
	OUT PVOID OutputBuffer OPTIONAL,
	IN ULONG OutputBufferLength,
	IN ULONG IoControlCode,
	OUT PIO_STATUS_BLOCK IoStatus,
	IN PDEVICE_OBJECT DeviceObject
)
{
	UNREFERENCED_PARAMETER(FileObject);
	UNREFERENCED_PARAMETER(Wait);
	UNREFERENCED_PARAMETER(InputBuffer);
	UNREFERENCED_PARAMETER(InputBufferLength);
	UNREFERENCED_PARAMETER(OutputBuffer);
	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(IoControlCode);
	UNREFERENCED_PARAMETER(IoStatus);
	UNREFERENCED_PARAMETER(DeviceObject);

	return TRUE;
}




/*++

Unload routine for CFB IrpDumper.

--*/
VOID _Function_class_(DRIVER_UNLOAD) DriverUnloadRoutine(PDRIVER_OBJECT DriverObject)
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
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverCloseRoutine(PDEVICE_OBJECT pObject, PIRP Irp)
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
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverCreateRoutine(PDEVICE_OBJECT pObject, PIRP Irp)
{
    UNREFERENCED_PARAMETER(pObject);

    PAGED_CODE();

    NTSTATUS Status = STATUS_UNSUCCESSFUL;

    KeAcquireInStackQueuedSpinLock(&SpinLockOwner, &SpinLockQueueOwner);
    
    if (pCurrentOwnerProcess == NULL )
    {
        pCurrentOwnerProcess = PsGetCurrentProcess();
        CfbDbgPrintOk(L"Locked device to process %p...\n", pCurrentOwnerProcess);
        Status = STATUS_SUCCESS;
    }   
	else if (PsGetCurrentProcess() == pCurrentOwnerProcess)
	{
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
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverDeviceControlRoutine(PDEVICE_OBJECT pObject, PIRP Irp)
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

	if (!NT_SUCCESS(Status))
	{
		CfbDbgPrintErr(L"IOCTL #%x returned %#x\n", IoctlCode, Status);
	}
	

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


/*++

--*/
BOOLEAN InterceptGenericFastIoRead(
	IN PFILE_OBJECT FileObject,
	IN PLARGE_INTEGER FileOffset,
	IN ULONG Length,
	IN BOOLEAN Wait,
	IN ULONG LockKey,
	OUT PVOID Buffer,
	OUT PIO_STATUS_BLOCK IoStatus,
	IN PDEVICE_OBJECT DeviceObject
)
{
	UNREFERENCED_PARAMETER(FileObject);
	UNREFERENCED_PARAMETER(FileOffset);
	UNREFERENCED_PARAMETER(Length);
	UNREFERENCED_PARAMETER(Wait);
	UNREFERENCED_PARAMETER(LockKey);
	UNREFERENCED_PARAMETER(Buffer);
	UNREFERENCED_PARAMETER(IoStatus);
	UNREFERENCED_PARAMETER(DeviceObject);

	return TRUE;
}


/*++

--*/
BOOLEAN InterceptGenericFastIoWrite(
	IN PFILE_OBJECT FileObject,
	IN PLARGE_INTEGER FileOffset,
	IN ULONG Length,
	IN BOOLEAN Wait,
	IN ULONG LockKey,
	OUT PVOID Buffer,
	OUT PIO_STATUS_BLOCK IoStatus,
	IN PDEVICE_OBJECT DeviceObject
)
{
	UNREFERENCED_PARAMETER(FileObject);
	UNREFERENCED_PARAMETER(FileOffset);
	UNREFERENCED_PARAMETER(Length);
	UNREFERENCED_PARAMETER(Wait);
	UNREFERENCED_PARAMETER(LockKey);
	UNREFERENCED_PARAMETER(Buffer);
	UNREFERENCED_PARAMETER(IoStatus);
	UNREFERENCED_PARAMETER(DeviceObject);

	return TRUE;
}
