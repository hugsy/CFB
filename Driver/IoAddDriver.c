#include "IoAddDriver.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HandleIoAddDriver)
#endif

#pragma auto_inline(off)


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
		/* IN KPROCESSOR_MODE */KernelMode,
		/* IN OUT PVOID */ NULL,
		/* OUT PVOID* */ (PVOID*)&pDriver);

	if (!NT_SUCCESS(status))
	{
		return STATUS_INVALID_PARAMETER;
	}

	//
	// check if driver is already hooked
	//

	if (IsDriverHooked(pDriver) || GetHookedDriverByName(lpDriverName))
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
NTSTATUS HandleIoAddDriver(PIRP Irp, PIO_STACK_LOCATION Stack)
{
	UNREFERENCED_PARAMETER(Irp);
	PAGED_CODE();

	CfbDbgPrint(L"Received 'IoctlAddDriver'\n");

	NTSTATUS Status = STATUS_SUCCESS;
	LPWSTR lpDriverName;
	ULONG InputBufferLen;

	do
	{

		//
		// Check IRP arguments
		//

		// lpDriverName = (LPWSTR)Irp->AssociatedIrp.SystemBuffer;
		lpDriverName = (LPWSTR)Stack->Parameters.DeviceIoControl.Type3InputBuffer;

		if (!lpDriverName)
		{
			Status = STATUS_UNSUCCESSFUL;
			break;
		}

		InputBufferLen = Stack->Parameters.DeviceIoControl.InputBufferLength;

		if (InputBufferLen >= HOOKED_DRIVER_MAX_NAME_LEN)
		{
			CfbDbgPrint(L"Input buffer too large\n");
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}


		//
		// Add the driver
		//
		Status = AddDriverByName(lpDriverName);

		CfbDbgPrint(L"AddDriverByName('%s') returned %#x\n", lpDriverName, Status);

	}
	while(0);

	return Status;
}

#pragma auto_inline()