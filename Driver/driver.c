#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>

#include "driver.h"

/**
 * Driver entry point: create the driver object for CFB
 */
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	DriverObject->DriverUnload = DriverUnloadRoutine;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateCloseRoutine;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateCloseRoutine;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDeviceControlRoutine;

	UNICODE_STRING name, symLink; 
	RtlInitUnicodeString(&name, CFB_DEVICE_NAME);
	RtlInitUnicodeString(&symLink, CFB_DEVICE_LINK);

	NTSTATUS status = STATUS_SUCCESS;

	PDEVICE_OBJECT DeviceObject;
	status = IoCreateDevice(DriverObject, 0, &name, FILE_DEVICE_UNKNOWN, 
							FILE_DEVICE_SECURE_OPEN, TRUE, &DeviceObject);
	if( !NT_SUCCESS(status) )
	{
		DbgPrint("Error creating device object (0x%08X)\n", status);
		return status;
	}

	status = IoCreateSymbolicLink(&symLink, &name);
	if (!NT_SUCCESS(status)) 
	{
		DbgPrint("Error creating symbolic link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);	
	} 
	else
	{
		DbgPrint("Device '%s' successfully created\n", CFB_DEVICE_NAME);
	}

	return status;
}


void DriverUnloadRoutine(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	return;
}


/**
 *
 */
NTSTATUS CompleteRequest(PIRP Irp, NTSTATUS status, ULONG_PTR Information) 
{
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = Information;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}


/**
 *
 */
NTSTATUS DriverCreateCloseRoutine(PDEVICE_OBJECT pObject, PIRP Irp) 
{
	UNREFERENCED_PARAMETER(pObject);

	return CompleteRequest(Irp, STATUS_SUCCESS, 0);
}


/**
 *
 */
NTSTATUS DriverDeviceControlRoutine(PDEVICE_OBJECT pObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pObject);
	UNREFERENCED_PARAMETER(Irp);

	NTSTATUS status = STATUS_SUCCESS;

	return status;
}