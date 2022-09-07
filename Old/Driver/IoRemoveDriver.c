#include "IoRemoveDriver.h"


extern PLIST_ENTRY g_HookedDriverHead;


/*++

--*/
NTSTATUS RemoveDriverByName(LPWSTR lpDriverName)
{
	CfbDbgPrintInfo(L"Removing driver '%s'\n", lpDriverName);

	PHOOKED_DRIVER pHookedDriverToRemove;
	NTSTATUS Status = GetHookedDriverByName(lpDriverName, &pHookedDriverToRemove);

	if (!NT_SUCCESS(Status))
	{
		CfbDbgPrintErr(L"No hooked driver found as '%s': Status=0x%x\n", lpDriverName, Status);
		return Status;
	}


	pHookedDriverToRemove->Enabled = FALSE;


	//
	// restore the former device control function pointer
	//
	PDRIVER_OBJECT pDriver = pHookedDriverToRemove->DriverObject;

	KeAcquireInStackQueuedSpinLock(&g_AddRemoveDriverSpinLock, &g_AddRemoveSpinLockQueue);

    for (DWORD i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
    {
        InterlockedExchangePointer(
            (PVOID)&pDriver->MajorFunction[i],
            (PVOID)pHookedDriverToRemove->OriginalRoutines[i]
        );
    }

	//
	// Restore Fast IO Dispatch pointers
	//
	
	if (pDriver->FastIoDispatch)
	{
		InterlockedExchangePointer(
			(PVOID)&pDriver->FastIoDispatch->FastIoDeviceControl,
			(PVOID)pHookedDriverToRemove->FastIoDeviceControl
		);

		InterlockedExchangePointer(
			(PVOID)&pDriver->FastIoDispatch->FastIoRead,
			(PVOID)pHookedDriverToRemove->FastIoRead
		);

		InterlockedExchangePointer(
			(PVOID)&pDriver->FastIoDispatch->FastIoWrite,
			(PVOID)pHookedDriverToRemove->FastIoWrite
		);
	}


	//
	// fix the chain
	//

    RemoveEntryList(&(pHookedDriverToRemove->ListEntry));
        
	KeReleaseInStackQueuedSpinLock(&g_AddRemoveSpinLockQueue);


	//
	// free the driver pool and its reference
	//
	ObDereferenceObject(pHookedDriverToRemove->DriverObject);

	ExFreePoolWithTag(pHookedDriverToRemove, CFB_DEVICE_TAG);

	return Status;
}


/*++

--*/
NTSTATUS RemoveAllDrivers()
{
	NTSTATUS Status = STATUS_SUCCESS;

    if (IsListEmpty(g_HookedDriverHead))
    {
		CfbDbgPrintInfo(L"Hooked driver list is empty, nothing to do\n");
        return Status;
    }
    
    UINT32 dwNbRemoved = 0;
    BOOLEAN bIsLast;
    PLIST_ENTRY Entry = g_HookedDriverHead->Flink;
    PLIST_ENTRY Next;

    do
	{
        Next = Entry->Flink;
        bIsLast = Next == g_HookedDriverHead;

        PHOOKED_DRIVER Driver = CONTAINING_RECORD(Entry, HOOKED_DRIVER, ListEntry);

		WCHAR OldDriverName[HOOKED_DRIVER_MAX_NAME_LEN]={ 0, };
		wcscpy_s( OldDriverName, HOOKED_DRIVER_MAX_NAME_LEN-sizeof(WCHAR), Driver->Name );
        
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

        Entry = Next;
    } 
	while ( !IsListEmpty(g_HookedDriverHead) );

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

