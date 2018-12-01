#include "IoAddDriver.h"

typedef enum
{
    Driver,
    FileSystem,
    Device
} OBJ_T;

extern PLIST_ENTRY g_HookedDriversHead;

/*++/

--*/
NTSTATUS AddObjectByName(LPWSTR lpObjectName, OBJ_T Type)
{
	NTSTATUS status = STATUS_SUCCESS;

	/* make sure the list is not full */
	if (GetNumberOfHookedDrivers() == CFB_MAX_HOOKED_DRIVERS)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	/* lookup the driver by name, and swap it if found */
	UNICODE_STRING UnicodeName;
    PDRIVER_OBJECT pDriver;
    PDEVICE_OBJECT pDevice;

    RtlInitUnicodeString(&UnicodeName, lpObjectName);

    switch (Type)
    {
    case Driver:
    case FileSystem:

        status = ObReferenceObjectByName(
            /* IN PUNICODE_STRING */ &UnicodeName,
            /* IN ULONG */ OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            /* IN PACCESS_STATE */ NULL,
            /* IN ACCESS_MASK */ 0,
            /* IN POBJECT_TYPE */ *IoDriverObjectType,
            /* IN KPROCESSOR_MODE */KernelMode,
            /* IN OUT PVOID */ NULL,
            /* OUT PVOID* */ (PVOID*)&pDriver
        );

        if (!NT_SUCCESS(status))
        {
            return status;
        }

        break;

    case Device:

        status = ObReferenceObjectByName(
            /* IN PUNICODE_STRING */ &UnicodeName,
            /* IN ULONG */ OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            /* IN PACCESS_STATE */ NULL,
            /* IN ACCESS_MASK */ 0,
            /* IN POBJECT_TYPE */ *IoDeviceObjectType,
            /* IN KPROCESSOR_MODE */KernelMode,
            /* IN OUT PVOID */ NULL,
            /* OUT PVOID* */ (PVOID*)&pDevice
        );

        if (!NT_SUCCESS(status))
        {
            return status;
        }

        pDriver = pDevice->DriverObject;
        
        break;

    default:
        return STATUS_INVALID_PARAMETER;
    }


	//
	// check if driver is already hooked
	//

	if ( IsDriverHooked(pDriver) )
	{
		return STATUS_ALREADY_REGISTERED;
	}


	//
	// create the new hooked driver pool object, and chain it to the rest
	//

	PHOOKED_DRIVER NewDriver = ExAllocatePoolWithTag(NonPagedPool, sizeof(HOOKED_DRIVER), CFB_DEVICE_TAG);
	if (!NewDriver)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlSecureZeroMemory(NewDriver, sizeof(HOOKED_DRIVER));


	CfbDbgPrintInfo( L"AddObjectByName('%s'): switching IRP_MJ_DEVICE_CONTROL with %p\n", lpObjectName, InterceptedDeviceControlRoutine );

	PVOID OldDeviceControlRoutine = InterlockedExchangePointer(
		(PVOID*)&pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL],
		(PVOID)InterceptedDeviceControlRoutine
	);


	CfbDbgPrintInfo( L"AddObjectByName('%s'): switching IRP_MJ_READ with %p\n", lpObjectName, InterceptedReadRoutine );

	PVOID OldReadRoutine = InterlockedExchangePointer(
		(PVOID*)&pDriver->MajorFunction[IRP_MJ_READ],
		(PVOID)InterceptedReadRoutine
	);


	CfbDbgPrintInfo( L"AddObjectByName('%s'): switching IRP_MJ_WRITE with %p\n", lpObjectName, InterceptedWriteRoutine );

	PVOID OldWriteRoutine = InterlockedExchangePointer(
		(PVOID*)&pDriver->MajorFunction[IRP_MJ_WRITE],
		(PVOID)InterceptedWriteRoutine
	);

	wcscpy_s(NewDriver->Name, sizeof(NewDriver->Name) / sizeof(wchar_t), lpObjectName);
	RtlUnicodeStringCopy(&NewDriver->UnicodeName, &UnicodeName);
	NewDriver->DriverObject = pDriver;
	NewDriver->OldDeviceControlRoutine = OldDeviceControlRoutine;
	NewDriver->OldReadRoutine = OldReadRoutine;
	NewDriver->OldWriteRoutine = OldWriteRoutine;
	NewDriver->Enabled = TRUE;
	

	//
	// add it to the list
	//

    InsertTailList(g_HookedDriversHead, &(NewDriver->ListEntry));

	return status;
}


/*++

--*/
NTSTATUS HandleIoAddDriver(PIRP Irp, PIO_STACK_LOCATION Stack)
{
	UNREFERENCED_PARAMETER(Irp);
	
	NTSTATUS Status = STATUS_SUCCESS;
	LPWSTR lpDriverName;
	ULONG InputBufferLen;

	do
	{

		//
		// Check IRP arguments
		//

		lpDriverName = (LPWSTR)Irp->AssociatedIrp.SystemBuffer;

		if (!lpDriverName)
		{
			Status = STATUS_INVALID_PARAMETER;
			break;
		}

		InputBufferLen = Stack->Parameters.DeviceIoControl.InputBufferLength;

		if (InputBufferLen >= HOOKED_DRIVER_MAX_NAME_LEN)
		{
			CfbDbgPrintErr(L"Input buffer too large\n");
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}


        CfbDbgPrintInfo(L"Adding AddObjectByName('%s') \n", lpDriverName);

		//
		// Add the driver
		//

        if (wcsncmp(lpDriverName, L"\\driver", 7) == 0)
        {
            Status = AddObjectByName(lpDriverName, Driver);
        }
        else if (wcsncmp(lpDriverName, L"\\filesystem", 11) == 0)
        {
            Status = AddObjectByName(lpDriverName, FileSystem);
        }
        else if (wcsncmp(lpDriverName, L"\\device", 7) == 0)
        {
            Status = AddObjectByName(lpDriverName, Device);
        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
        }
		

		CfbDbgPrintOk(L"AddObjectByName('%s') returned %#x\n", lpDriverName, Status);

	}
	while(0);

	return Status;
}


