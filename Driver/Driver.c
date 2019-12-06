#include "Driver.h"


#ifdef ALLOC_PRAGMA

//
// Discard after initialization
//
#pragma alloc_text (INIT, DriverEntry)

#endif

UINT16 g_dwNumberOfHandle;

static PDEVICE_OBJECT g_DeviceObject;
//static IO_REMOVE_LOCK g_DriverRemoveLock;
//static KSPIN_LOCK g_SpinLock;
//static KLOCK_QUEUE_HANDLE g_SpinLockQueue;

//
// Not more than one process can interact with the device object
//
static PEPROCESS pCurrentOwnerProcess;
static KSPIN_LOCK SpinLockOwner;
static KLOCK_QUEUE_HANDLE SpinLockQueueOwner;

extern PLIST_ENTRY g_HookedDriverHead;



/*++

Routine Description:

This routine is called when trying to ReadFile() from a handle to the device IrpDumper. In a regular scenario,
Broker will call ReadFile() on the device only when it was notified some new data was available. However, if
the broker tries to read regardless of the event, the function returns successfully with any data back to userland.

The data sent back to the client is continuously following the sequence: header, inputbuffer, outputbuffer

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
		return CompleteRequest(Irp, Status, 0);


	if ( BufferSize == 0 )
	{
		//
		// If BufferSize == 0, the client is probing for the size of the IRP raw data to allocate.
		//
		return CompleteRequest(Irp, STATUS_SUCCESS, dwExpectedSize);
	}

	if ( BufferSize != dwExpectedSize )
	{
		CfbDbgPrintErr( L"Buffer size is invalid, expected %dB, got %dB\n", dwExpectedSize, BufferSize );
		return CompleteRequest(Irp, STATUS_INFO_LENGTH_MISMATCH, dwExpectedSize);
	}

	NT_ASSERT(Irp->MdlAddress);

	UINT32 BufferOffset = 0;
	PVOID Buffer = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, HighPagePriority );
	if ( !Buffer )
		return CompleteRequest(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
		

	Status = PopFromQueue(&pInterceptedIrp);

	if ( !NT_SUCCESS(Status) )
		return CompleteRequest(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);

	//
	// Copy header
	//
	PINTERCEPTED_IRP_HEADER pInterceptedIrpHeader = pInterceptedIrp->Header;
	RtlCopyMemory( Buffer, pInterceptedIrpHeader, sizeof(INTERCEPTED_IRP_HEADER) );
	BufferOffset += sizeof(INTERCEPTED_IRP_HEADER);


	//
	// Copy the IRP input buffer (if any)
	//
	if(pInterceptedIrpHeader->InputBufferLength && pInterceptedIrp->InputBuffer)
	{
		ULONG_PTR RawBuffer = ((ULONG_PTR)Buffer) + BufferOffset;
		RtlCopyMemory((PVOID)RawBuffer, pInterceptedIrp->InputBuffer, pInterceptedIrpHeader->InputBufferLength);
		BufferOffset += pInterceptedIrpHeader->InputBufferLength;
	}

	//
	// Copy the IRP output buffer (if any)
	//
	if (pInterceptedIrpHeader->OutputBufferLength && pInterceptedIrp->OutputBuffer)
	{		
		ULONG_PTR RawBuffer = ((ULONG_PTR)Buffer) + BufferOffset;
		CfbDbgPrintInfo(L"RawBuffer+%d=%p <- OutputBufferLength=%x, OutputBuffer=%p\n", BufferOffset, RawBuffer, pInterceptedIrpHeader->OutputBufferLength, pInterceptedIrp->OutputBuffer);
		RtlCopyMemory((PVOID)RawBuffer, pInterceptedIrp->OutputBuffer, pInterceptedIrpHeader->OutputBufferLength);
		BufferOffset += pInterceptedIrpHeader->OutputBufferLength;
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
	UNREFERENCED_PARAMETER(Irp);

	PAGED_CODE();

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

	CfbDbgLogInit();

	CfbDbgPrintInfo(L"Loading driver IrpDumper\n");
	pCurrentOwnerProcess = NULL;

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
		FALSE,
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
	g_dwNumberOfHandle = 0;
	//IoInitializeRemoveLock(&g_DriverRemoveLock, CFB_DEVICE_TAG, 0, 0);
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

	- DeviceObject

	- Irp


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
#ifndef PDRIVER_DISPATCH
typedef NTSTATUS(*PDRIVER_DISPATCH)(IN PDEVICE_OBJECT DeviceObject, OUT PIRP Irp);
#endif

NTSTATUS InterceptGenericRoutine(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp)
{ 

	PHOOKED_DRIVER curDriver = GetHookedDriverFromDeviceObject(DeviceObject);
	if (!curDriver)
	{
		//
		// This is really bad: it means our interception routine got called by a non-hooked driver
		// Could be a bad pointer restoration. Anyway, we log and fail for now.
		//

		CfbDbgPrintErr(
			L"Failed to find a HOOKED_DRIVER object associated to the received IRP.\n"
			L"This could indicates a corruption of the hooked driver list, you should probably reboot...\n"
		);
		return CompleteRequest( Irp, STATUS_NO_SUCH_DEVICE, 0 );
	} 


	//
	// Capture the IRP data
	//
	PINTERCEPTED_IRP pIrpInfo = NULL;
	if (IsMonitoringEnabled() && curDriver->Enabled == TRUE && pCurrentOwnerProcess != PsGetCurrentProcess())
	{
		NTSTATUS Status = HandleInterceptedIrp(curDriver, DeviceObject, Irp, &pIrpInfo);

		if (!NT_SUCCESS(Status) && Status != STATUS_NOT_IMPLEMENTED)
		{
			CfbDbgPrintWarn(L"HandleInterceptedIrp() failed (status=0x%X)\n", Status);
		}
	}

	//
	// And call the original routine
	//
	PVOID UserBuffer = Irp->UserBuffer;
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);
	PDRIVER_DISPATCH OriginalIoctlDeviceControl = curDriver->OriginalRoutines[Stack->MajorFunction];
	NTSTATUS IoctlStatus = OriginalIoctlDeviceControl(DeviceObject, Irp);


	//
	// Collect the result from the result
	//
	if (IoctlStatus != STATUS_PENDING &&
		IsMonitoringEnabled() && 
		curDriver->Enabled == TRUE && 
		pCurrentOwnerProcess != PsGetCurrentProcess() &&
		pIrpInfo != NULL
	)
	{
		NTSTATUS Status = CompleteHandleInterceptedIrp(Stack, UserBuffer, IoctlStatus, pIrpInfo);
		if (!NT_SUCCESS(Status))
			CfbDbgPrintWarn(L"CompleteHandleInterceptedIrp() failed, Status=0x%x\n", Status);
	}


	//
	// Push the new irp to the queue and notify the broker
	//
	if (pIrpInfo)
	{
		NTSTATUS Status = PushToQueue(pIrpInfo);
		if (!NT_SUCCESS(Status))
		{
			CfbDbgPrintErr(L"PushToQueue(%p) failed, status=0x%x\n", pIrpInfo, Status);
			FreeInterceptedIrp(pIrpInfo);
		}
		else
		{
			SetNewIrpInQueueAlert();
			CfbDbgPrintOk(L"pushed IRPs[%d] = %p\n", GetIrpListSize() ? GetIrpListSize() - 1 : 0, pIrpInfo);
		}
	}

	return IoctlStatus;
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
	// Unswap all hooked drivers left to avoid new IRPs to be handled by the driver
	//

	KeEnterCriticalRegion();
	DisableMonitoring();
	ReleaseTestCaseStructures();
	RemoveAllDrivers();
	KeLeaveCriticalRegion();


	//
	// Disable events and clear the queue
	//

	ClearNotificationPointer();

	FlushQueue();

	//
	// Delete the device object
	//

	UNICODE_STRING symLink = RTL_CONSTANT_STRING(CFB_DEVICE_LINK);

	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);

	CfbDbgPrintOk(L"Success unloading '%s'\n", CFB_PROGRAM_NAME_SHORT);

	CfbDbgLogFree();
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
NTSTATUS _Function_class_(DRIVER_DISPATCH) DriverCloseRoutine(_In_ PDEVICE_OBJECT Device, _In_ PIRP Irp)
{
	UNREFERENCED_PARAMETER(Device);

	PAGED_CODE();

    KeAcquireInStackQueuedSpinLock(&SpinLockOwner, &SpinLockQueueOwner);
	
	g_dwNumberOfHandle--;

    if (g_dwNumberOfHandle == 0 && pCurrentOwnerProcess != NULL)
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
	PIO_STACK_LOCATION lpStack = IoGetCurrentIrpStackLocation(Irp);
	PIO_SECURITY_CONTEXT lpSecurityContext = lpStack->Parameters.Create.SecurityContext;


	//
	// Ensure the calling process has SeDebugPrivilege
	//
	PPRIVILEGE_SET lpRequiredPrivileges;
	UCHAR ucPrivilegesBuffer[FIELD_OFFSET(PRIVILEGE_SET, Privilege) + sizeof(LUID_AND_ATTRIBUTES)] = { 0 };

	lpRequiredPrivileges = (PPRIVILEGE_SET)ucPrivilegesBuffer;
	lpRequiredPrivileges->PrivilegeCount = 1;
	lpRequiredPrivileges->Control = PRIVILEGE_SET_ALL_NECESSARY;
	lpRequiredPrivileges->Privilege[0].Luid.LowPart = SE_DEBUG_PRIVILEGE;
	lpRequiredPrivileges->Privilege[0].Luid.HighPart = 0;
	lpRequiredPrivileges->Privilege[0].Attributes = 0;

	if (!SePrivilegeCheck(
		lpRequiredPrivileges,
		&lpSecurityContext->AccessState->SubjectSecurityContext,
		Irp->RequestorMode
		)
	)
	{
		Status = STATUS_PRIVILEGE_NOT_HELD;
	}
	else
	{
		PEPROCESS pCallingProcess = PsGetCurrentProcess();

		KeAcquireInStackQueuedSpinLock(&SpinLockOwner, &SpinLockQueueOwner);
    
		if (pCurrentOwnerProcess == NULL )
		{
			//
			// if there's no process owner, affect one and increment the handle counter
			//
			pCurrentOwnerProcess = pCallingProcess;
			CfbDbgPrintOk(L"Locked device to EPROCESS=%p...\n", pCurrentOwnerProcess);
			g_dwNumberOfHandle++;
			Status = STATUS_SUCCESS;
		}   
		else if (pCallingProcess == pCurrentOwnerProcess)
		{
			//
			// if the CreateFile() originates from the owner process, increment the handle counter
			//
			g_dwNumberOfHandle++;
			Status = STATUS_SUCCESS;
		}
		else
		{
			//
			// in any other case, simply reject
			//
			Status = STATUS_DEVICE_ALREADY_ATTACHED;
		}

		KeReleaseInStackQueuedSpinLock(&SpinLockQueueOwner);
	}

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

	//
	// this should never happen as we checked the process when getting the handle, but still
	//
	if (pCurrentOwnerProcess != PsGetCurrentProcess())
		return CompleteRequest(Irp, STATUS_DEVICE_ALREADY_ATTACHED, 0);


	NTSTATUS Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION CurrentStack = IoGetCurrentIrpStackLocation(Irp);
	ULONG IoctlCode = CurrentStack->Parameters.DeviceIoControl.IoControlCode;

	
	switch (IoctlCode)
	{

	case IOCTL_AddDriver:
		Status = HandleIoAddDriver(Irp, CurrentStack);
		break;

	case IOCTL_RemoveDriver:
		Status = HandleIoRemoveDriver(Irp, CurrentStack);
		break;

	case IOCTL_EnableMonitoring:
		Status = HandleIoEnableMonitoring(Irp, CurrentStack );
		break;

	case IOCTL_DisableMonitoring:
		Status = HandleIoDisableMonitoring(Irp, CurrentStack );
		break;

	case IOCTL_GetNumberOfDrivers:
		Status = HandleIoGetNumberOfHookedDrivers(Irp, CurrentStack);
		break;

	case IOCTL_GetDriverInfo:
		Status = HandleIoGetDriverInfo( Irp, CurrentStack );
		break;

	case IOCTL_SetEventPointer:
		Status = HandleIoSetEventPointer(Irp, CurrentStack);
		break;

    case IOCTL_StoreTestCase:
        Status = HandleIoStoreTestCase(Irp, CurrentStack);
        break;

	default:
		CfbDbgPrintErr(L"Received invalid ioctl code 0x%X\n", IoctlCode);
		Status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	if (!NT_SUCCESS(Status))
		CfbDbgPrintErr(L"IOCTL #%x returned %#x\n", IoctlCode, Status);

	
	return CompleteRequest(Irp, Status, 0);
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


/*++

Routine Description:

The InterceptGenericFastIoRoutinePre() routine wrapper intercepts FastIOs input data.


Arguments:

	- DeviceObject
	- InputBuffer
	- InputBufferLength
	- IoControlCode
	- pIrpOut


Return Value:

	Returns TRUE on success.

--*/
BOOLEAN 
InterceptGenericFastIoRoutine(
	_In_ PDEVICE_OBJECT DeviceObject,
	_In_ UINT32 Type,
	_In_ PVOID Buffer,
	_In_ ULONG BufferLength,
	_In_ ULONG IoControlCode,
	_In_ UINT32 Flags,
	_Inout_ PINTERCEPTED_IRP *pIrpOut
)
{
	PHOOKED_DRIVER Driver = GetHookedDriverFromDeviceObject(DeviceObject);
	if (!Driver)
	{
		CfbDbgPrintErr(
			L"Failed to find driver for InterceptGenericFastIoRoutine(). "
			L"This could mean a corrupted state of " CFB_DRIVER_NAME L". "
			L"You should reboot to avoid further corruption...\n"
		);
		return FALSE;
	}

	NTSTATUS Status = STATUS_SUCCESS;

	if (IsMonitoringEnabled() && Driver->Enabled)
	{
		Status = HandleInterceptedFastIo(
			Driver,
			DeviceObject,
			Type,
			IoControlCode,
			Buffer,
			BufferLength,
			Flags,
			&*pIrpOut
		);
	}

	return Status == STATUS_SUCCESS;
}


/*++

Routine Description:

The InterceptGenericFastIoDeviceControl() interception routine wrapper.

```c
	typedef BOOLEAN (*PFAST_IO_DEVICE_CONTROL) (
		IN struct _FILE_OBJECT *FileObject,
		IN BOOLEAN Wait,
		IN PVOID InputBuffer OPTIONAL,
		IN ULONG InputBufferLength,
		OUT PVOID OutputBuffer OPTIONAL,
		IN ULONG OutputBufferLength,
		IN ULONG IoControlCode,
		OUT PIO_STATUS_BLOCK IoStatus,
		IN struct _DEVICE_OBJECT *DeviceObject
	);
```

Arguments:
	- FileObject
	- Wait
	- InputBuffer
	- InputBufferLength
	- OutputBuffer
	- OutputBufferLength
	- IoControlCode
	- IoStatus
	- DeviceObject

Return Value:

	Returns TRUE on success.

--*/
BOOLEAN InterceptGenericFastIoDeviceControl(
	IN PFILE_OBJECT FileObject,
	IN BOOLEAN Wait,
	IN PVOID InputBuffer OPTIONAL,
	IN ULONG InputBufferLength,
	OUT PVOID OutputBuffer OPTIONAL,
	IN ULONG OutputBufferLength,
	IN ULONG IoControlCode,
	OUT PIO_STATUS_BLOCK IoStatus,
	IN PDEVICE_OBJECT DeviceObject
)
{
	PINTERCEPTED_IRP pIrp = NULL;

	//
	// Capture the input
	//
	if (!InterceptGenericFastIoRoutine(
		DeviceObject, 
		CFB_INTERCEPTED_IRP_TYPE_FASTIO_IOCTL,
		InputBuffer, 
		InputBufferLength, 
		IoControlCode, 
		CFB_FASTIO_USE_INPUT_BUFFER | CFB_FASTIO_INIT_QUEUE_MESSAGE,
		&pIrp)
	)
		return FALSE;
	
	PHOOKED_DRIVER Driver = GetHookedDriverFromDeviceObject(DeviceObject);
	PFAST_IO_DEVICE_CONTROL OriginalFastIoDeviceControl = Driver->FastIoDeviceControl;
	BOOLEAN bRes = OriginalFastIoDeviceControl(
		FileObject, 
		Wait, 
		InputBuffer, 
		InputBufferLength, 
		OutputBuffer, 
		OutputBufferLength, 
		IoControlCode, 
		IoStatus, 
		DeviceObject
	);
	
	//
	// Capture the output - pIrp was already initialized
	//
	if (!InterceptGenericFastIoRoutine(
		DeviceObject, 
		CFB_INTERCEPTED_IRP_TYPE_FASTIO_IOCTL,
		OutputBuffer, 
		OutputBufferLength, 
		IoControlCode, 
		CFB_FASTIO_USE_OUTPUT_BUFFER,
		&pIrp
	))
		return FALSE;

	return bRes;
}


/*++

Routine Description:

The InterceptGenericFastIoRead() interception routine wrapper.

```c
	typedef BOOLEAN (*PFAST_IO_READ) (
		IN PFILE_OBJECT FileObject,
		IN PLARGE_INTEGER FileOffset,
		IN ULONG Length,
		IN BOOLEAN Wait,
		IN ULONG LockKey,
		OUT PVOID Buffer,
		OUT PIO_STATUS_BLOCK IoStatus,
		IN PDEVICE_OBJECT DeviceObject
	);
```

Arguments:
	- FileObject
	- FileOffset
	- Length
	- Wait
	- LockKey
	- Buffer
	- IoStatus
	- DeviceObject

Return Value:

	Returns TRUE on success.

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
	PINTERCEPTED_IRP pIrp = NULL;
	PHOOKED_DRIVER Driver = GetHookedDriverFromDeviceObject(DeviceObject);
	if (!Driver)
		return FALSE;

	PFAST_IO_READ OriginalFastIoRead = Driver->FastIoRead;
	BOOLEAN bRes = OriginalFastIoRead(
		FileObject,
		FileOffset,
		Length,
		Wait,
		LockKey,
		Buffer,
		IoStatus,
		DeviceObject
	);

	
	if (!InterceptGenericFastIoRoutine(
		DeviceObject, 
		CFB_INTERCEPTED_IRP_TYPE_FASTIO_READ,
		Buffer, 
		Length, 
		(ULONG)-1,
		CFB_FASTIO_USE_OUTPUT_BUFFER | CFB_FASTIO_INIT_QUEUE_MESSAGE,
		&pIrp
	))
		return FALSE;
	
	return bRes;
}


/*++

Routine Description:

The InterceptGenericFastIoWrite() interception routine wrapper.

```c
	typedef BOOLEAN (*PFAST_IO_WRITE) (
		IN PFILE_OBJECT FileObject,
		IN PLARGE_INTEGER FileOffset,
		IN ULONG Length,
		IN BOOLEAN Wait,
		IN ULONG LockKey,
		OUT PVOID Buffer,
		OUT PIO_STATUS_BLOCK IoStatus,
		IN PDEVICE_OBJECT DeviceObject
	);
```

Arguments:
	- FileObject
	- FileOffset
	- Length
	- Wait
	- LockKey
	- Buffer
	- IoStatus
	- DeviceObject

Return Value:

	Returns TRUE on success.

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
	PINTERCEPTED_IRP pIrp = NULL;
	if (!InterceptGenericFastIoRoutine(
		DeviceObject, 
		CFB_INTERCEPTED_IRP_TYPE_FASTIO_WRITE,
		Buffer, 
		Length, 
		(ULONG)-1,
		CFB_FASTIO_USE_INPUT_BUFFER | CFB_FASTIO_INIT_QUEUE_MESSAGE,
		&pIrp
	))
		return FALSE;

	PHOOKED_DRIVER Driver = GetHookedDriverFromDeviceObject(DeviceObject);
	PFAST_IO_WRITE OriginalFastIoWrite = Driver->FastIoWrite;
	return OriginalFastIoWrite(
		FileObject,
		FileOffset,
		Length,
		Wait,
		LockKey,
		Buffer,
		IoStatus,
		DeviceObject
	);
}
