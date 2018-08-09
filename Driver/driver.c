#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include <string.h>
#include <ntstrsafe.h>

#include "driver.h"
#include "ioctls.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text (INIT, DriverEntry)
#pragma alloc_text (PAGE, DriverUnloadRoutine)
#pragma alloc_text (PAGE, DriverCreateCloseRoutine)
#pragma alloc_text (PAGE, DriverDeviceControlRoutine)
#endif


/*++

--*/
VOID CfbDbgPrint(const char* lpFormatString, ...)
{
#ifdef _DEBUG
	va_list args;
	char buffer[1024] = { 0, };
	va_start(args, lpFormatString);
	vsprintf_s(buffer, sizeof(buffer) / sizeof(char), lpFormatString, args);
	va_end(args);

	KdPrint(("[CFB] " ));
	KdPrint((buffer));
#endif
}


/*++

--*/
SIZE_T GetNumberOfHookedDrivers()
{
	SIZE_T i;
	PHOOKED_DRIVER ptr;
	for (i = 0, ptr = g_HookedDriversHead; ptr; ptr = ptr->Next, i++);
	return i;
}


/*++

--*/
PHOOKED_DRIVER GetLastHookedDriver()
{
	PHOOKED_DRIVER lastDriver;
	for (lastDriver = g_HookedDriversHead; lastDriver; lastDriver = lastDriver->Next) {}
	return lastDriver;
}


/*++

--*/
BOOLEAN IsDriverHooked(PDRIVER_OBJECT pObj)
{
	PHOOKED_DRIVER ptr;
	for (ptr = g_HookedDriversHead; ptr; ptr = ptr->Next)
	{
		if (ptr->DriverObject == pObj)
		{
			return TRUE;
		}
	}
	return FALSE;
}


/*++

--*/
PHOOKED_DRIVER GetHookedDriverByName(LPWSTR lpDriverName)
{
	PHOOKED_DRIVER Driver = g_HookedDriversHead;

	while (Driver)
	{
		if (wcscmp(Driver->Name, lpDriverName) == 0)
			return Driver;

		Driver = Driver->Next;
	}

	return NULL;
}


/*++/

--*/
NTSTATUS AddDriverByName(LPWSTR lpDriverName)
{
	NTSTATUS status = STATUS_SUCCESS;

	/* make sure the list is not full */
	if (GetNumberOfHookedDrivers() == CFB_MAX_DEVICES)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	/* lookup the driver by name, and swap it if found */
	UNICODE_STRING UnicodeName;
	RtlInitUnicodeString(&UnicodeName, lpDriverName);

	PDRIVER_OBJECT pDriver;
	status = ObReferenceObjectByName( /* IN PUNICODE_STRING */ &UnicodeName,
												/* IN ULONG */ OBJ_CASE_INSENSITIVE,
												/* IN PACCESS_STATE */ NULL,
												/* IN ACCESS_MASK */ 0,
												/* IN POBJECT_TYPE */ *IoDriverObjectType,
												/* IN KPROCESSOR_MODE */KernelMode ,
												/* IN OUT PVOID */ NULL,
												/* OUT PVOID* */ (PVOID*)&pDriver);

	if (!NT_SUCCESS(status))
	{
		return STATUS_INVALID_ADDRESS;
	}

	/* check if driver is already hooked */
	if( IsDriverHooked(pDriver) || GetHookedDriverByName(lpDriverName))
	{
		return STATUS_ALREADY_REGISTERED;
	}


	/* create the new hooked driver object */
	PHOOKED_DRIVER NewDriver = ExAllocatePoolWithTag(PagedPool, sizeof(HOOKED_DRIVER), CFB_DEVICE_TAG);
	if (!NewDriver)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlSecureZeroMemory(NewDriver, sizeof(HOOKED_DRIVER));

	PVOID OldDeviceControlRoutine = InterlockedExchangePointer((PVOID*)&pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL],
		(PVOID)DummyHookedDispatchRoutine);

	wcscpy_s(NewDriver->Name, sizeof(NewDriver->Name) / sizeof(wchar_t), lpDriverName);
	RtlUnicodeStringCopy(&NewDriver->UnicodeName, &UnicodeName);
	NewDriver->DriverObject = pDriver;
	NewDriver->OldDeviceControlRoutine = OldDeviceControlRoutine;
	NewDriver->Enabled = TRUE;
	NewDriver->Next = NULL;


	/* add it to the list */
	PHOOKED_DRIVER LastDriver = GetLastHookedDriver();
	if (LastDriver == NULL)
	{
		/* 1st element */
		g_HookedDriversHead = NewDriver;
	}
	else
	{
		LastDriver->Next = NewDriver;
	}

	return status;
}


/*++
--*/
NTSTATUS RemoveDriverByName(LPWSTR lpDriverName)
{
	NTSTATUS status = STATUS_SUCCESS;

	PHOOKED_DRIVER DriverToRemove, PrevDriverToRemove = NULL;

	for (DriverToRemove = g_HookedDriversHead;
		DriverToRemove && !wcscmp(DriverToRemove->Name, lpDriverName);
		DriverToRemove = DriverToRemove->Next)
	{
		PrevDriverToRemove = DriverToRemove;
	}

	if (DriverToRemove == NULL)
	{
		return STATUS_UNSUCCESSFUL;
	}


	DriverToRemove->Enabled = FALSE;


	/* restore the former device control function pointer */
	PDRIVER_OBJECT pDriver;
	status = ObReferenceObjectByName(
		/* IN PUNICODE_STRING */ &DriverToRemove->UnicodeName,
		/* IN ULONG */ OBJ_CASE_INSENSITIVE,
		/* IN PACCESS_STATE */ NULL,
		/* IN ACCESS_MASK */ 0,
		/* IN POBJECT_TYPE */ *IoDriverObjectType,
		/* IN KPROCESSOR_MODE */KernelMode,
		/* IN OUT PVOID */ NULL,
		/* OUT PVOID* */ (PVOID*)&pDriver);

	if ( !NT_SUCCESS(status) )
	{
		return STATUS_INVALID_PARAMETER;
	}

	/* reverse back the device control major function */
	InterlockedExchangePointer((PVOID*)DummyHookedDispatchRoutine,
		(PVOID)&pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL]);

	/* fix the chain */
	if (PrevDriverToRemove == NULL)
	{
		/* trying to remove 1st element */
		g_HookedDriversHead = DriverToRemove->Next;
	}
	else
	{
		PrevDriverToRemove->Next = DriverToRemove->Next;
	}


	/* free the driver */
	ObDereferenceObject(DriverToRemove->DriverObject);
	ExFreePoolWithTag(DriverToRemove, CFB_DEVICE_TAG);

	return status;
}


/*++

Driver entry point: create the driver object for CFB

--*/
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	PAGED_CODE();

	CfbDbgPrint("-----------------------------------------\n");
	CfbDbgPrint("     Loading driver %wZ \n", CFB_PROGRAM_NAME_SHORT);
	CfbDbgPrint("-----------------------------------------\n\n");

	DriverObject->DriverUnload = DriverUnloadRoutine;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCreateCloseRoutine;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateCloseRoutine;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDeviceControlRoutine;

	UNICODE_STRING name, symLink;
	RtlInitUnicodeString(&name, CFB_DEVICE_NAME);
	RtlInitUnicodeString(&symLink, CFB_DEVICE_LINK);

	NTSTATUS status = STATUS_SUCCESS;

	PDEVICE_OBJECT DeviceObject;
	status = IoCreateDevice(DriverObject,
							0,
							&name,
							FILE_DEVICE_UNKNOWN,
							FILE_DEVICE_SECURE_OPEN,
							TRUE,
							&DeviceObject);

	if( !NT_SUCCESS(status) )
	{
		CfbDbgPrint("Error creating device object (0x%08X)\n", status);
		return status;
	}

	status = IoCreateSymbolicLink(&symLink, &name);
	if (!NT_SUCCESS(status))
	{
		CfbDbgPrint("Error creating symbolic link (0x%08X)\n", status);
		IoDeleteDevice(DeviceObject);
	}
	else
	{
		CfbDbgPrint("Device '%wZ' successfully created\n", CFB_DEVICE_NAME);
		g_HookedDriversHead = NULL;
	}

	return status;
}




/**
 * struct _IRP: https://msdn.microsoft.com/en-us/library/windows/hardware/ff550694(v=vs.85).aspx
 */
NTSTATUS DummyHookedDispatchRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{

	PAGED_CODE();

	CfbDbgPrint("TODO !! I'm hooked !!\n\n");

	/* Find driver in list */
	PHOOKED_DRIVER curDriver = g_HookedDriversHead;
	BOOLEAN Found = FALSE;

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
		return STATUS_INVALID_PARAMETER;
	}

	/* Call the original routine */
	typedef NTSTATUS(*PDRIVER_DISPATCH)(PDEVICE_OBJECT DeviceObject, PIRP Irp);
	PDRIVER_DISPATCH (*OldDispatchRoutine) = (PDRIVER_DISPATCH*)curDriver->OldDeviceControlRoutine;

	return (*OldDispatchRoutine)(DeviceObject, Irp);
}

/*++

--*/
VOID DriverUnloadRoutine(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	CfbDbgPrint("Unloading %wZ\n\n", CFB_PROGRAM_NAME_SHORT);


	/* Unlink all HookedDrivers left */

	PHOOKED_DRIVER Driver;

	while ( (Driver = GetLastHookedDriver()) != NULL )
	{
		NTSTATUS status = RemoveDriverByName(Driver->Name);
		if (!NT_SUCCESS(status))
		{
			CfbDbgPrint("Failed to remove driver %wZ\n", Driver->Name);
		}
		else
		{
			CfbDbgPrint("Driver %wZ removed\n", Driver->Name);
		}
	}

	CfbDbgPrint("Success unloading %wZ removed\n", CFB_PROGRAM_NAME_SHORT);

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

	PAGED_CODE();

	return CompleteRequest(Irp, STATUS_SUCCESS, 0);
}


/**
 *
 */
NTSTATUS DriverDeviceControlRoutine(PDEVICE_OBJECT pObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(pObject);

	PAGED_CODE();

	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION CurrentStack = IoGetCurrentIrpStackLocation(Irp);
	/*
	ULONG InputLen = CurrentStack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG OutputLen = CurrentStack->Parameters.DeviceIoControl.OutputBufferLength;
	*/
	ULONG IoctlCode = CurrentStack->Parameters.DeviceIoControl.IoControlCode;

	switch (IoctlCode)
	{
	case IOCTL_AddDriver:
		DbgPrint("Received 'IoctlAddDriver'\n");
		break;

	case IOCTL_RemoveDriver:
		DbgPrint("Received 'IoctlRemoveDriver'\n");
		break;

	case IOCTL_GetNumberOfDrivers:
		DbgPrint("Received 'IoctlGetNumberOfDrivers'\n");
		break;

	case IOCTL_ListAllDrivers:
		DbgPrint("Received 'IoctlListAllDrivers'\n");
		break;

	default:
		DbgPrint("Received invalid ioctl '%x'\n", IoctlCode);
		return STATUS_UNSUCCESSFUL;
	}

	return status;
}
