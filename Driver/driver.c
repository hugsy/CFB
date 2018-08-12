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
PHOOKED_DRIVER GetPreviousHookedDriver(PHOOKED_DRIVER pDriver)
{
	if (!g_HookedDriversHead)
		return NULL;

	PHOOKED_DRIVER pCurDriver = g_HookedDriversHead->Next;
	PHOOKED_DRIVER pPrevDriver = g_HookedDriversHead;
	BOOLEAN Found = FALSE;

	while (pCurDriver)
	{
		if (pCurDriver == pDriver)
		{
			Found = TRUE;
			break;
		}

		pPrevDriver = pCurDriver;
		pCurDriver = pCurDriver->Next;
	}

	//
	// if not found
	//
	if (!Found)
		return NULL;

	return pPrevDriver;
}


/*++

--*/
PHOOKED_DRIVER GetNextHookedDriver(PHOOKED_DRIVER pDriver)
{
	if (!pDriver)
		return NULL;

	return pDriver->Next;
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

	//
	// check if driver is already hooked
	//

	if( IsDriverHooked(pDriver) || GetHookedDriverByName(lpDriverName))
	{
		return STATUS_ALREADY_REGISTERED;
	}


	//
	// create the new hooked driver pool object, and chain it to the rest
	//

	PHOOKED_DRIVER NewDriver = ExAllocatePoolWithTag(PagedPool, sizeof(HOOKED_DRIVER), CFB_DEVICE_TAG);
	if (!NewDriver)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlSecureZeroMemory(NewDriver, sizeof(HOOKED_DRIVER));


	PVOID OldDeviceControlRoutine = InterlockedExchangePointer(
		(PVOID*)&pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL],
		(PVOID)InterceptedDispatchRoutine
	);

	wcscpy_s(NewDriver->Name, sizeof(NewDriver->Name) / sizeof(wchar_t), lpDriverName);
	RtlUnicodeStringCopy(&NewDriver->UnicodeName, &UnicodeName);
	NewDriver->DriverObject = pDriver;
	NewDriver->OldDeviceControlRoutine = OldDeviceControlRoutine;
	NewDriver->Enabled = TRUE;
	NewDriver->Next = NULL;


	//
	// add it to the list
	//
	PHOOKED_DRIVER LastDriver = GetLastHookedDriver();
	if (LastDriver == NULL)
	{
		g_HookedDriversHead = NewDriver; // 1st element
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

	CfbDbgPrint(L"RemoveDriverByName('%s')\n", lpDriverName);

	PHOOKED_DRIVER pDriverToRemove = GetHookedDriverByName(lpDriverName);

	if (!pDriverToRemove)
	{
		CfbDbgPrint(L"RemoveDriverByName(): no hooked driver found as '%s'\n", lpDriverName);
		return STATUS_INVALID_PARAMETER;
	}

	PHOOKED_DRIVER pPrevDriverToRemove = GetPreviousHookedDriver(pDriverToRemove);


	//
	// restore the former device control function pointer
	//
	PDRIVER_OBJECT pDriver = pDriverToRemove->DriverObject;

	CfbDbgPrint(L"RemoveDriverByName('%s'): restoring IRP_MJ_DEVICE_CONTROL\n", lpDriverName);

	pDriverToRemove->Enabled = FALSE;

	InterlockedExchangePointer(
		(PVOID*)&pDriverToRemove->OldDeviceControlRoutine,
		(PVOID)&pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL]
	);


	//
	// fix the chain
	//

	CfbDbgPrint(L"RemoveDriverByName('%s'): unlink %p with %p\n", lpDriverName, pDriverToRemove, pPrevDriverToRemove);

	if (pPrevDriverToRemove == NULL)
	{
		g_HookedDriversHead = pDriverToRemove->Next;  // trying to remove 1st element
	}
	else
	{
		pPrevDriverToRemove->Next = pDriverToRemove->Next;
	}


	//
	// free the driver pool and its reference
	//
	ObDereferenceObject(pDriverToRemove->DriverObject);
	ExFreePoolWithTag(pDriverToRemove, CFB_DEVICE_TAG);

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
		return STATUS_NO_SUCH_DEVICE;
	}


	//
	// TODO: collect IRP data here(only if Enabled)
	//



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


	//
	// Delete the device object
	//

	UNICODE_STRING symLink = { 0, };
	RtlInitUnicodeString(&symLink, CFB_DEVICE_LINK);

	IoDeleteSymbolicLink(&symLink);
	IoDeleteDevice(DriverObject->DeviceObject);

	CfbDbgPrint(L"Success unloading %s removed\n", CFB_PROGRAM_NAME_SHORT);

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
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		lpDriverName = (LPWSTR)Irp->AssociatedIrp.SystemBuffer;

		if (IoctlCode == IOCTL_AddDriver)
		{
			CfbDbgPrint(L"Received 'IoctlAddDriver'\n");
			Status = AddDriverByName(lpDriverName);
			CfbDbgPrint(L"AddDriverByName('%s') returned %#x\n", lpDriverName, Status);
		}
		else
		{
			CfbDbgPrint(L"Received 'IoctlRemoveDriver'\n");
			Status = RemoveDriverByName(lpDriverName);
			CfbDbgPrint(L"RemoveDriverByName('%s') returned %#x\n", lpDriverName, Status);
		}
		break;


	case IOCTL_GetNumberOfDrivers:
		CfbDbgPrint(L"Received 'IoctlGetNumberOfDrivers'\n");

		OutputBufferLen = CurrentStack->Parameters.DeviceIoControl.OutputBufferLength;
		if (OutputBufferLen < sizeof(DWORD))
		{
			Status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		DWORD* dwRes = (DWORD*)Irp->AssociatedIrp.SystemBuffer;
		*dwRes = GetNumberOfHookedDrivers();
		Status = STATUS_SUCCESS;
		break;


	case IOCTL_GetDriverInfo:
		CfbDbgPrint(L"Received 'IoctlGetDriverInfo'\n");

		InputBufferLen = CurrentStack->Parameters.DeviceIoControl.InputBufferLength;

		if (InputBufferLen > sizeof(DWORD))
		{
			CfbDbgPrint(L"Input buffer too large\n");
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		OutputBufferLen = CurrentStack->Parameters.DeviceIoControl.OutputBufferLength;
		if (OutputBufferLen < sizeof(HOOKED_DRIVER_INFO))
		{
			Status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		DWORD* pdwDriverIndex = (DWORD*)Irp->AssociatedIrp.SystemBuffer;

		Status = GetDriverInfo(*pdwDriverIndex, (PHOOKED_DRIVER_INFO)Irp->AssociatedIrp.SystemBuffer);

		CfbDbgPrint(L"GetDriverInfo(%d) returned %#x\n", *pdwDriverIndex, Status);

		break;


	default:
		CfbDbgPrint(L"Received invalid ioctl '%#X'\n", IoctlCode);
		Status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	CompleteRequest(Irp, Status, Information);

	return Status;
}
