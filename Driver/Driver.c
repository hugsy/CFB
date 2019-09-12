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

Routine Description:

This routine is called when trying to ReadFile() from a handle to the device IrpDumper.


Arguments:

	DeviceObject - a pointer to the Device Object being closed

	Irp - a pointer to the IRP context


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverReadRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp )
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PAGED_CODE();

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	PIO_STACK_LOCATION pStack = IoGetCurrentIrpStackLocation( Irp );

	if ( !pStack )
	{
		CfbDbgPrintErr( L"IoGetCurrentIrpStackLocation() failed (IRP %p)\n", Irp );
		return CompleteRequest(Irp, STATUS_UNSUCCESSFUL, 0);
	}


	ULONG BufferSize = pStack->Parameters.Read.Length;

	PINTERCEPTED_IRP pInterceptedIrp;
	UINT32 dwExpectedSize;

	Status = PeekHeadEntryExpectedSize(&dwExpectedSize);


	if (!NT_SUCCESS(Status))
	{

		if (Status != STATUS_NO_MORE_ENTRIES)
			return CompleteRequest(Irp, Status, 0);


        //
        // Second chance
        //
        KeWaitForSingleObject(g_EventNotificationPointer, UserRequest, KernelMode, FALSE, NULL);
        Status = PeekHeadEntryExpectedSize(&dwExpectedSize);
        if (!NT_SUCCESS(Status))
			return CompleteRequest(Irp, Status, 0);

	}


	if ( BufferSize == 0 )
	{
		//
		// If BufferSize == 0, the client is probing for the size of the IRP raw data to allocate
		// We end the IRP and announce the expected size
		//
		
		return CompleteRequest(Irp, STATUS_BUFFER_TOO_SMALL, dwExpectedSize);
	}


	if ( BufferSize != dwExpectedSize )
	{
		CfbDbgPrintErr( L"Buffer is too small, expected %dB, got %dB\n", dwExpectedSize, BufferSize );
		return CompleteRequest(Irp, STATUS_INFO_LENGTH_MISMATCH, 0);
	}


	PVOID Buffer = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, HighPagePriority );

	if ( !Buffer )
		return CompleteRequest(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);


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

	return CompleteRequest(Irp, STATUS_SUCCESS, dwExpectedSize);
}


/*++

Routine Description:

Generic routine for implemented major types.


Arguments:

	DeviceObject - a pointer to the Device Object being closed

	Irp - a pointer to the IRP context


Return Value:

	Returns STATUS_NOT_IMPLEMENTED.

--*/
NTSTATUS 
_Function_class_(DRIVER_DISPATCH) 
IrpNotImplementedHandler(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PAGED_CODE();

#ifdef _DEBUG
	CfbDbgPrintInfo(L"MajorFunction = 0x%x\n", IoGetCurrentIrpStackLocation(Irp)->MajorFunction);
#endif

	return CompleteRequest(Irp, STATUS_NOT_IMPLEMENTED, 0); 
}


/*++

Routine Description:

The cleanup is invoked when the last handle to the device object is being closed for a specific process.
We must cleanup the context associated with that handle for said process.


Arguments:

	DeviceObject - a pointer to the Device Object being closed

	Irp - a pointer to the IRP context


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS 
_Function_class_(DRIVER_DISPATCH) 
DriverCleanup(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp )
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



	//
	// Context is clean, let's exit gracefully.
	//

	return CompleteRequest(Irp, STATUS_SUCCESS, 0);
}


/*++

Routine Description:

The driver entry point function for the IrpDumper driver: it will create the device object for CFB
stored in a global, and associate all the necessary routines to the driver object.

Last, it will initialize all the structures internal to the driver.


Arguments:

	DeviceObject - a pointer to the Device Object being closed

	Irp - a pointer to the IRP context


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS 
DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	PAGED_CODE();

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	CfbDbgPrintInfo(L"Loading driver IrpDumper\n");

	UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(CFB_DEVICE_NAME); 
	UNICODE_STRING DeviceSymlink = RTL_CONSTANT_STRING(CFB_DEVICE_LINK);


	//
	// Create the device object
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

	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateRoutine;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCloseRoutine;
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

Routine Description:

This routine is the CFB interception routine that will be executed *before* the original
routine from the hooked driver.

Links:
 - https://msdn.microsoft.com/en-us/library/windows/hardware/ff550694(v=vs.85).aspx


Arguments:




Return Value:

	Returns STATUS_SUCCESS on success.

--*/
typedef NTSTATUS(*PDRIVER_DISPATCH)(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp);

NTSTATUS InterceptGenericRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
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
		PDRIVER_DISPATCH OldRoutine = (PDRIVER_DISPATCH)curDriver->OriginalRoutines[dwIndex];
		Status = OldRoutine(DeviceObject, Irp);
	}

	IoReleaseRemoveLock( &g_DriverRemoveLock, Irp );

	return Status;
}




/*++

Routine Description:

Unload routine for CFB IrpDumper.


Arguments:

	- DriverObject


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
VOID _Function_class_(DRIVER_UNLOAD) DriverUnloadRoutine(_In_ PDRIVER_OBJECT DriverObject)
{  

	//
	// Delete the device object
	//

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(CFB_DEVICE_LINK);

	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);

	CfbDbgPrintOk(L"Success unloading '%s'\n", CFB_PROGRAM_NAME_SHORT);

	return;
}


/*++

Routine Description:

Generic function for IRP completion


Arguments:

	- Irp

	- Status

	- Information


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS CompleteRequest(_In_ PIRP Irp, _In_ NTSTATUS Status, _In_ ULONG_PTR Information)
{
	Irp->IoStatus.Status = Status;
	Irp->IoStatus.Information = Information;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return Status;
}


/*++

Routine Description:


Arguments:


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverCloseRoutine(_In_ PDEVICE_OBJECT pObject, _In_ PIRP Irp)
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

Routine Description:


Arguments:


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverCreateRoutine(_In_ PDEVICE_OBJECT pObject, _In_ PIRP Irp)
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

Routine Description:

This routine handles the dispatch of DeviceIoControl() sent to a IrpDumper device object. It will 
parse the IRP and invoke the routine associated to the specific IOCTL code.


Arguments:

	- DeviceObject

	- Irp


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverDeviceControlRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG_PTR Information = 0;


    if ( pCurrentOwnerProcess != PsGetCurrentProcess() )
        return CompleteRequest(Irp, STATUS_DEVICE_ALREADY_ATTACHED, Information);

	
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
		Status = HandleIoEnableMonitoring(Irp, CurrentStack );
		break;


	case IOCTL_DisableMonitoring:
		CfbDbgPrintInfo( L"Received 'IoctlDisableDriver'\n" );
		Status = HandleIoDisableMonitoring(Irp, CurrentStack );
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
		//CfbDbgPrintInfo(L"Received 'IoctlStoreTestCase'\n");
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
	
	return CompleteRequest(Irp, Status, Information);
}


/*++

Routine Description:

The DeviceIoControl() interception routine wrapper.


Arguments:

	- DeviceObject

	- Irp


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS InterceptedDeviceControlRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp )
{
	return InterceptGenericRoutine(DeviceObject, Irp);
}


/*++

Routine Description:

The ReadFile() interception routine wrapper.


Arguments:

	- DeviceObject

	- Irp


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS InterceptedReadRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp )
{
	return InterceptGenericRoutine(DeviceObject, Irp);
}
	

/*++

Routine Description:

The WriteFile() interception routine wrapper.


Arguments:

	- DeviceObject
	
	- Irp


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS InterceptedWriteRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp )
{
	return InterceptGenericRoutine(DeviceObject, Irp);
}

