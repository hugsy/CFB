#include "IoRemoveDriver.h"


extern PLIST_ENTRY g_HookedDriversHead;


/*++

--*/
NTSTATUS RemoveDriverByName(LPWSTR lpDriverName)
{
	NTSTATUS status = STATUS_SUCCESS;

	CfbDbgPrintInfo(L"Removing driver '%s'\n", lpDriverName);

	PHOOKED_DRIVER pDriverToRemove;
	NTSTATUS Status = GetHookedDriverByName(lpDriverName, &pDriverToRemove);

	if (!NT_SUCCESS(Status))
	{
		CfbDbgPrintErr(L"No hooked driver found as '%s': Status=0x%x\n", lpDriverName, Status);
		return STATUS_INVALID_PARAMETER;
	}


	pDriverToRemove->Enabled = FALSE;


	//
	// restore the former device control function pointer
	//
	PDRIVER_OBJECT pDriver = pDriverToRemove->DriverObject;

	CfbDbgPrintInfo(L"- '%s' - restoring IRP_MJ_DEVICE_CONTROL to %p\n", lpDriverName, pDriverToRemove->OldDeviceControlRoutine);

	InterlockedExchangePointer(
		(PVOID)&pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL],
		(PVOID)pDriverToRemove->OldDeviceControlRoutine
	);


	CfbDbgPrintInfo( L"- '%s' - restoring IRP_MJ_READ to %p\n", lpDriverName, pDriverToRemove->OldReadRoutine );

	InterlockedExchangePointer(
		(PVOID)&pDriver->MajorFunction[IRP_MJ_READ],
		(PVOID)pDriverToRemove->OldReadRoutine
	);


	CfbDbgPrintInfo( L"- '%s' - restoring IRP_MJ_WRITE to %p\n", lpDriverName, pDriverToRemove->OldWriteRoutine );

	InterlockedExchangePointer(
		(PVOID)&pDriver->MajorFunction[IRP_MJ_WRITE],
		(PVOID)pDriverToRemove->OldWriteRoutine
	);


	//
	// fix the chain
	//

    RemoveEntryList(&(pDriverToRemove->ListEntry));
        


	//
	// free the driver pool and its reference
	//
	ObDereferenceObject(pDriverToRemove->DriverObject);

	ExFreePoolWithTag(pDriverToRemove, CFB_DEVICE_TAG);

	return status;
}


/*++

--*/
NTSTATUS RemoveAllDrivers()
{
	NTSTATUS Status = STATUS_SUCCESS;

    if (IsListEmpty(g_HookedDriversHead))
    {
		CfbDbgPrintInfo(L"Hooked driver list is empty, nothing to do\n");
        return Status;
    }
    
    UINT32 dwNbRemoved = 0;
    BOOLEAN bIsLast;
    PLIST_ENTRY Entry = g_HookedDriversHead->Flink;

    do
	{
        bIsLast = Entry->Flink == g_HookedDriversHead;

        PHOOKED_DRIVER Driver = CONTAINING_RECORD(Entry, HOOKED_DRIVER, ListEntry);

		WCHAR OldDriverName[HOOKED_DRIVER_MAX_NAME_LEN]={ 0, };
		wcscpy( OldDriverName, Driver->Name );

		Status = RemoveDriverByName( Driver->Name );
		if (!NT_SUCCESS(Status))
		{
			CfbDbgPrintErr(L"Failed to remove driver %s\n", OldDriverName );
		}
		else
		{
			CfbDbgPrintOk(L"Driver %s removed\n", OldDriverName );
			dwNbRemoved++;
		}

        if (bIsLast)
        {
            break;
        }

        Entry = Entry->Flink;

    } 
	while ( !IsListEmpty(g_HookedDriversHead) );

	CfbDbgPrintOk(L"Removed %lu drivers\n", dwNbRemoved);

	return Status;
}


/*++

--*/
NTSTATUS HandleIoRemoveDriver(PIRP Irp, PIO_STACK_LOCATION Stack)
{
	UNREFERENCED_PARAMETER(Irp);
	PAGED_CODE();

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


		//
		// Remove the driver
		//
		Status = RemoveDriverByName(lpDriverName);

		CfbDbgPrintOk(L"RemoveDriverByName('%s') returned %#x\n", lpDriverName, Status);

	} while (0);

	return Status;
}

