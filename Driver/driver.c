#include <ntifs.h>
#include <ntddk.h>
#include <wdm.h>
#include <ntstrsafe.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>


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
VOID CfbDbgPrint(const wchar_t* lpFormatString, ...)
{
#ifdef _DEBUG
	va_list args;
	wchar_t buffer[1024] = { 0, };
	va_start(args, lpFormatString);
	vswprintf_s(buffer, sizeof(buffer) / sizeof(wchar_t), lpFormatString, args);
	va_end(args);

	// todo add timestamp
	KdPrint(("[CFB] %S", buffer));
#endif
}


/*++

--*/
DWORD GetNumberOfHookedDrivers()
{
	DWORD i;
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


/*++

--*/
PHOOKED_DRIVER GetHookedDriverByIndex(DWORD dwIndex)
{
	if (dwIndex >= GetNumberOfHookedDrivers())
	{
		CfbDbgPrint(L"Cannot reach index %ld\n", dwIndex);
		return NULL;
	}

	PHOOKED_DRIVER Driver = g_HookedDriversHead;

	for (; dwIndex; dwIndex--, Driver = Driver->Next);

	return Driver;
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
		return STATUS_INVALID_PARAMETER;
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


--*/
NTSTATUS GetDriverInfo(DWORD dwIndex, PHOOKED_DRIVER_INFO pDrvInfo)
{
	NTSTATUS status = STATUS_SUCCESS;

	PHOOKED_DRIVER pDriver = GetHookedDriverByIndex(dwIndex);

	if (!pDriver)
	{
		return STATUS_INVALID_PARAMETER;
	}

	__try
	{
		RtlSecureZeroMemory(pDrvInfo, sizeof(HOOKED_DRIVER_INFO));
		RtlCopyMemory((PVOID)pDrvInfo->Enabled, (PVOID)pDriver->Enabled, sizeof(BOOLEAN));
		RtlCopyMemory((PVOID)pDrvInfo->Name, (PVOID)pDriver->Name, HOOKED_DRIVER_MAX_NAME_LEN * sizeof(WCHAR));
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		status = GetExceptionCode();
		CfbDbgPrint(L"[-] Exception Code: 0x%X\n", status);
	}

	return status;
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
		CfbDbgPrint(L"Error creating device object (0x%08X)\n", status);
		return status;
	}

	CfbDbgPrint(L"Device object '%s' created\n", CFB_DEVICE_NAME);

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

Link struct _IRP: https://msdn.microsoft.com/en-us/library/windows/hardware/ff550694(v=vs.85).aspx

--*/
NTSTATUS DummyHookedDispatchRoutine(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{

	PAGED_CODE();

	CfbDbgPrint(L"TODO !! I'm hooked !!\n\n");

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

	CfbDbgPrint(L"Unloading %s\n", CFB_PROGRAM_NAME_SHORT);


	/* Unlink all HookedDrivers left */

	PHOOKED_DRIVER Driver;
	DWORD dwNbRemoved = 0, dwNbLoaded = GetNumberOfHookedDrivers();

	while ( (Driver = GetLastHookedDriver()) != NULL )
	{
		NTSTATUS status = RemoveDriverByName(Driver->Name);
		if (!NT_SUCCESS(status))
		{
			CfbDbgPrint(L"Failed to remove driver %s\n", Driver->Name);
		}
		else
		{
			CfbDbgPrint(L"Driver %s removed\n", Driver->Name);
			dwNbRemoved++;
		}
	}

	CfbDbgPrint(L"Removed %lu/%lu drivers\n", dwNbRemoved, dwNbLoaded);

	g_HookedDriversHead = NULL;


	/* Delete the device object */

	UNICODE_STRING symLink;
	RtlInitUnicodeString(&symLink, CFB_DEVICE_LINK);

	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);

	CfbDbgPrint(L"Success unloading %s removed\n", CFB_PROGRAM_NAME_SHORT);

	return;
}


/*++

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

	NTSTATUS status = STATUS_SUCCESS;

	PIO_STACK_LOCATION CurrentStack = IoGetCurrentIrpStackLocation(Irp);

	ULONG IoctlCode = CurrentStack->Parameters.DeviceIoControl.IoControlCode;

	LPWSTR lpDriverName;
	ULONG InputBufferLen, OutputBufferLen;

	switch (IoctlCode)
	{
	case IOCTL_AddDriver:
	case IOCTL_RemoveDriver:
		InputBufferLen = CurrentStack->Parameters.DeviceIoControl.InputBufferLength;

		if (InputBufferLen >= HOOKED_DRIVER_MAX_NAME_LEN)
		{
			CfbDbgPrint(L"Input buffer too large\n");
			status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		lpDriverName = (LPWSTR)Irp->AssociatedIrp.SystemBuffer;
		lpDriverName[InputBufferLen - 1] = L'\0';

		if (IoctlCode == IOCTL_AddDriver)
		{
			CfbDbgPrint(L"Received 'IoctlAddDriver'\n");
			status = AddDriverByName(lpDriverName);
			CfbDbgPrint(L"AddDriverByName('%s') returned %#x\n", lpDriverName, status);
		}
		else
		{
			CfbDbgPrint(L"Received 'IoctlRemoveDriver'\n");
			status = RemoveDriverByName(lpDriverName);
			CfbDbgPrint(L"RemoveDriverByName('%s') returned %#x\n", lpDriverName, status);
		}
		break;


	case IOCTL_GetNumberOfDrivers:
		CfbDbgPrint(L"Received 'IoctlGetNumberOfDrivers'\n");

		OutputBufferLen = CurrentStack->Parameters.DeviceIoControl.OutputBufferLength;
		if (OutputBufferLen < sizeof(SIZE_T))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		DWORD* dwRes = (DWORD*)Irp->AssociatedIrp.SystemBuffer;
		*dwRes = GetNumberOfHookedDrivers();
		status = STATUS_SUCCESS;
		break;


	case IOCTL_GetDriverInfo:
		CfbDbgPrint(L"Received 'IoctlGetDriverInfo' (TODO)\n");

		InputBufferLen = CurrentStack->Parameters.DeviceIoControl.InputBufferLength;

		if (InputBufferLen > sizeof(DWORD))
		{
			CfbDbgPrint(L"Input buffer too large\n");
			status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		OutputBufferLen = CurrentStack->Parameters.DeviceIoControl.OutputBufferLength;
		if (OutputBufferLen < sizeof(HOOKED_DRIVER_INFO))
		{
			status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		DWORD* pdwDriverIndex = (DWORD*)Irp->AssociatedIrp.SystemBuffer;

		status = GetDriverInfo(*pdwDriverIndex, (PHOOKED_DRIVER_INFO)Irp->AssociatedIrp.SystemBuffer);

		CfbDbgPrint(L"GetDriverInfo(%d) returned %#x\n", *pdwDriverIndex, status);

		break;


	default:
		CfbDbgPrint(L"Received invalid ioctl '%#X'\n", IoctlCode);
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	return status;
}
