#include "IoRemoveDriver.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HandleIoRemoveDriver)
#endif

#pragma auto_inline(off)


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

	CfbDbgPrint(L"RemoveDriverByName('%s'): restoring IRP_MJ_DEVICE_CONTROL to %p\n", lpDriverName, pDriverToRemove->OldDeviceControlRoutine);

	pDriverToRemove->Enabled = FALSE;

	InterlockedExchangePointer(
		(PVOID)&pDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL],
		(PVOID)pDriverToRemove->OldDeviceControlRoutine
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
NTSTATUS RemoveAllDrivers()
{
	NTSTATUS Status = STATUS_SUCCESS;

	PHOOKED_DRIVER Driver;
	UINT32 dwNbRemoved = 0, dwNbLoaded = GetNumberOfHookedDrivers();

	while ((Driver = GetLastHookedDriver()) != NULL)
	{
		NTSTATUS status = RemoveDriverByName(Driver->Name);
		if (!NT_SUCCESS(status))
		{
			CfbDbgPrint(L"Failed to remove driver %s\n", Driver->Name);
			Status = status;
		}
		else
		{
			CfbDbgPrint(L"Driver %s removed\n", Driver->Name);
			dwNbRemoved++;
		}
	}

	CfbDbgPrint(L"Removed %lu/%lu drivers\n", dwNbRemoved, dwNbLoaded);

	if (dwNbRemoved != dwNbLoaded)
	{
		CfbDbgPrint(L"[!] Not all hooked structures were successfully deallocated, the system might be unstable. It is recommended to reboot.\n");
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

	CfbDbgPrint(L"Received 'IoctlAddDriver'\n");

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
			CfbDbgPrint(L"Input buffer too large\n");
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}


		//
		// Add the driver
		//
		Status = RemoveDriverByName(lpDriverName);

		CfbDbgPrint(L"AddDriverByName('%s') returned %#x\n", lpDriverName, Status);

	} while (0);

	return Status;
}

#pragma auto_inline()
