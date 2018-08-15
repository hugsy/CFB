#include "Common.h"

#include "Driver.h"
#include "IoctlCodes.h"
#include "Utils.h"
#include "HookedDrivers.h"
#include "PipeComm.h"

#include "IoAddDriver.h"
#include "IoRemoveDriver.h"
#include "IoGetDriverInfo.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, DriverUnloadRoutine)
#pragma alloc_text (PAGE, DriverCreateCloseRoutine)
#pragma alloc_text (PAGE, DriverDeviceControlRoutine)
#endif



/*++

--*/
NTSTATUS IrpNotImplementedHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	PAGED_CODE();
	CfbDbgPrint(L"In IrpNotImplementedHandler()\n");
	CompleteRequest(Irp, STATUS_SUCCESS, 0);
	return STATUS_NOT_SUPPORTED;
}


/*++

Driver entry point: create the driver object for CFB

--*/
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	PAGED_CODE();

	CfbDbgPrint(L"-----------------------------------------\n");
	CfbDbgPrint(L"     Loading driver %s\n", CFB_PROGRAM_NAME_SHORT);
	CfbDbgPrint(L"-----------------------------------------\n");

	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PDEVICE_OBJECT DeviceObject;
	UNICODE_STRING name, symLink;

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
		CfbDbgPrint(L"Error creating device object (0x%08X)\n", status);
		if (DeviceObject)
		{
			IoDeleteDevice(DeviceObject);
		}

		return status;
	}

	//
	// Populate the IRP handlers
	//

	for (DWORD i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
	{
		DriverObject->MajorFunction[i] = IrpNotImplementedHandler;
	}

	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateCloseRoutine;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateCloseRoutine;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDeviceControlRoutine;

	DriverObject->DriverUnload = DriverUnloadRoutine;

	CfbDbgPrint(L"Device object '%s' created\n", CFB_DEVICE_NAME);

	//
	// Create the symlink
	//
	status = IoCreateSymbolicLink(&symLink, &name);
	if (!NT_SUCCESS(status))
	{
		CfbDbgPrint(L"Error creating symbolic link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);
	}
	else
	{
		CfbDbgPrint(L"Symlink '%s' to device object created\n", CFB_DEVICE_LINK);
		g_HookedDriversHead = NULL;
	}

	CfbDbgPrint(L"Device '%s' successfully created\n", CFB_DEVICE_NAME);
	return status;
}



/*++

This routine is the CFB interception routine that will be executed *before* the original
routine from the hooked driver.

Links:
 - https://msdn.microsoft.com/en-us/library/windows/hardware/ff550694(v=vs.85).aspx

--*/
typedef NTSTATUS(*PDRIVER_DISPATCH)(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NTSTATUS InterceptedDispatchRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{

	PAGED_CODE();

	CfbDbgPrint(L"In InterceptedDispatchRoutine(%p, %p)\n", DeviceObject, Irp);

	//
	// Find the original DEVICE_CONTROL function for the driver
	//
	PHOOKED_DRIVER curDriver = g_HookedDriversHead;
	BOOLEAN Found = FALSE;
	NTSTATUS Status;
	PIO_STACK_LOCATION Stack;

	while (curDriver)
	{
		if (curDriver->DriverObject == DeviceObject->DriverObject)
		{
			Found = TRUE;
			break;
		}

		curDriver = curDriver->Next;
	}

	if (!Found)
	{
		//
		// This is really bad: it means our interception routine got called by a non-hooked driver
		// Could be a bad pointer restoration. Anyway, we log and fail for now.
		//
		CfbDbgPrint(L"InterceptedDispatchRoutine() failed: couldn't get the current DriverObject\n");
		Status = STATUS_NO_SUCH_DEVICE;

		CompleteRequest(Irp, Status, 0);
		return Status;
	}


	//
	// Push the message to the named pipe
	//
	Stack = IoGetCurrentIrpStackLocation(Irp);
	Status = HandleInterceptedIrp(Irp, Stack);

	//if (!NT_SUCCESS(Status))
	{
		CfbDbgPrintErr(L"HandleInterceptedIrp() returned %#X\n", Status);
	}



	//
	// Call the original routine
	//

	CfbDbgPrint(L"Found OriginalDriver = '%s', calling %p\n", curDriver->Name, curDriver->OldDeviceControlRoutine);

	PDRIVER_DISPATCH OldDispatchRoutine = (DRIVER_DISPATCH*)curDriver->OldDeviceControlRoutine;
	return OldDispatchRoutine(DeviceObject, Irp);
}


/*++

--*/
VOID DriverUnloadRoutine(PDRIVER_OBJECT DriverObject)
{
	PAGED_CODE();

	CfbDbgPrint(L"Unloading %s\n", CFB_PROGRAM_NAME_SHORT);


	//
	// Unlink all HookedDrivers left
	//

	RemoveAllDrivers();


	//
	// Delete the device object
	//

	UNICODE_STRING symLink = { 0, };
	RtlInitUnicodeString(&symLink, CFB_DEVICE_LINK);

	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);

	CfbDbgPrint(L"Success unloading %s\n", CFB_PROGRAM_NAME_SHORT);

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
NTSTATUS DriverCreateCloseRoutine(PDEVICE_OBJECT pObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pObject);

	PAGED_CODE();

	return CompleteRequest(Irp, STATUS_SUCCESS, 0);
}


/*++

--*/
NTSTATUS DriverDeviceControlRoutine(PDEVICE_OBJECT pObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pObject);

	PAGED_CODE();

	NTSTATUS Status = STATUS_SUCCESS;
	ULONG_PTR Information = 0;

	PIO_STACK_LOCATION CurrentStack = IoGetCurrentIrpStackLocation(Irp);
	ULONG IoctlCode = CurrentStack->Parameters.DeviceIoControl.IoControlCode;

	switch (IoctlCode)
	{

	case IOCTL_AddDriver:
		CfbDbgPrint(L"Received 'IoctlGetNumberOfDrivers'\n");
		Status = HandleIoAddDriver(Irp, CurrentStack);
		break;


	case IOCTL_RemoveDriver:
		CfbDbgPrint(L"Received 'IoctlRemoveDriver'\n");
		Status = HandleIoRemoveDriver(Irp, CurrentStack);
		break;

	// TODO : add ioctl to remove all drivers


	case IOCTL_GetNumberOfDrivers:
		CfbDbgPrint(L"Received 'IoctlGetNumberOfDrivers'\n");
		Status = HandleIoGetNumberOfHookedDrivers(Irp, CurrentStack);
		break;


	case IOCTL_GetDriverInfo:
		CfbDbgPrint(L"Received 'IoctlGetDriverInfo'\n");
		Status = HandleIoGetNumberOfHookedDrivers(Irp, CurrentStack);
		break;


	default:
		CfbDbgPrint(L"Received invalid ioctl '%#X'\n", IoctlCode);
		Status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	CfbDbgPrint(L"IOCTL #%x returned %#x\n", IoctlCode, Status);

	CompleteRequest(Irp, Status, Information);

	return Status;
}
