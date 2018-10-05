#include "IoRemoveDriver.h"




/*++

--*/
NTSTATUS RemoveDriverByName(LPWSTR lpDriverName)
{
	NTSTATUS status = STATUS_SUCCESS;

	CfbDbgPrintInfo(L"RemoveDriverByName('%s')\n", lpDriverName);

	PHOOKED_DRIVER pDriverToRemove = GetHookedDriverByName(lpDriverName);

	if (!pDriverToRemove)
	{
		CfbDbgPrintErr(L"RemoveDriverByName(): no hooked driver found as '%s'\n", lpDriverName);
		return STATUS_INVALID_PARAMETER;
	}

	PHOOKED_DRIVER pPrevDriverToRemove = GetPreviousHookedDriver(pDriverToRemove);


	pDriverToRemove->Enabled = FALSE;


	//
	// restore the former device control function pointer
	//
	PDRIVER_OBJECT pDriver = pDriverToRemove->DriverObject;

	CfbDbgPrintInfo(L"RemoveDriverByName('%s'): restoring IRP_MJ_DEVICE_CONTROL to %p\n", lpDriverName, pDriverToRemove->OldDeviceControlRoutine);

	InterlockedExchangePointer(
		(PVOID)&pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL],
		(PVOID)pDriverToRemove->OldDeviceControlRoutine
	);


	CfbDbgPrintInfo( L"RemoveDriverByName('%s'): restoring IRP_MJ_READ to %p\n", lpDriverName, pDriverToRemove->OldReadRoutine );

	InterlockedExchangePointer(
		(PVOID)&pDriver->MajorFunction[IRP_MJ_READ],
		(PVOID)pDriverToRemove->OldReadRoutine
	);


	CfbDbgPrintInfo( L"RemoveDriverByName('%s'): restoring IRP_MJ_WRITE to %p\n", lpDriverName, pDriverToRemove->OldWriteRoutine );

	InterlockedExchangePointer(
		(PVOID)&pDriver->MajorFunction[IRP_MJ_WRITE],
		(PVOID)pDriverToRemove->OldWriteRoutine
	);


	//
	// fix the chain
	//

	CfbDbgPrintInfo(L"RemoveDriverByName('%s'): unlink %p with %p\n", lpDriverName, pDriverToRemove, pPrevDriverToRemove);

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
NTSTATUS RemoveAllDrivers()
{
	NTSTATUS Status = STATUS_SUCCESS;

	PHOOKED_DRIVER Driver;
	UINT32 dwNbRemoved = 0, dwNbLoaded = GetNumberOfHookedDrivers();

	while ((Driver = GetLastHookedDriver()) != NULL)
	{

		WCHAR OldDriverName[HOOKED_DRIVER_MAX_NAME_LEN]={ 0, };
		wcscpy( OldDriverName, Driver->Name );

		NTSTATUS status = RemoveDriverByName( Driver->Name );
		if (!NT_SUCCESS(status))
		{
			CfbDbgPrintErr(L"Failed to remove driver %s\n", OldDriverName );
			Status = status;
		}
		else
		{
			CfbDbgPrintOk(L"Driver %s removed\n", OldDriverName );
			dwNbRemoved++;
		}
	}

	CfbDbgPrintOk(L"Removed %lu/%lu drivers\n", dwNbRemoved, dwNbLoaded);

	if (dwNbRemoved != dwNbLoaded)
	{
		CfbDbgPrintErr(L"Not all hooked structures were successfully deallocated, the system might be unstable. It is recommended to reboot.\n");
	}

	g_HookedDriversHead = NULL;

	return Status;
}


/*++

--*/
NTSTATUS HandleIoRemoveDriver(PIRP Irp, PIO_STACK_LOCATION Stack)
{
	UNREFERENCED_PARAMETER(Irp);
	PAGED_CODE();

	CfbDbgPrintInfo(L"In 'HandleIoRemoveDriver'\n");

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
			Status = STATUS_UNSUCCESSFUL;
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
		// Add the driver
		//
		Status = RemoveDriverByName(lpDriverName);

		CfbDbgPrintOk(L"RemoveDriverByName('%s') returned %#x\n", lpDriverName, Status);

	} while (0);

	return Status;
}

