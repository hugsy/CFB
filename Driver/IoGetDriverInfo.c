#include "IoGetDriverInfo.h"



/*++


--*/
static NTSTATUS GetDriverInfo(_In_ PHOOKED_DRIVER pHookedDriver, _Out_ PHOOKED_DRIVER_INFO pDrvInfo, _Out_ PULONG pdwNbWrittenBytes)
{

	NTSTATUS Status = STATUS_SUCCESS;

	__try
	{
		RtlSecureZeroMemory(pDrvInfo, sizeof(HOOKED_DRIVER_INFO));
		pDrvInfo->Enabled = pHookedDriver->Enabled;
		pDrvInfo->DriverAddress = (ULONG_PTR)pHookedDriver->DriverObject;
		pDrvInfo->NumberOfRequestIntercepted = pHookedDriver->NumberOfRequestIntercepted;
		wcscpy_s(pDrvInfo->Name, pHookedDriver->DriverObject->DriverName.Length, pHookedDriver->Name);
		*pdwNbWrittenBytes = sizeof(HOOKED_DRIVER_INFO);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		Status = GetExceptionCode();
		CfbDbgPrintErr(L"[-] Exception Code: 0x%X\n", Status);
	}

	return Status;
}



/*++

--*/
NTSTATUS HandleIoGetDriverInfo(_In_ PIRP Irp, _Inout_ PIO_STACK_LOCATION Stack, _Out_ PULONG pdwDataWritten)
{
	UNREFERENCED_PARAMETER(Irp);
	PAGED_CODE();

	NTSTATUS Status = STATUS_SUCCESS;

	do
	{
        PHOOKED_DRIVER_INFO lpDriverInfo = (PHOOKED_DRIVER_INFO)Irp->AssociatedIrp.SystemBuffer;
		if (!lpDriverInfo)
		{
			Status = STATUS_INVALID_PARAMETER;
			break;
		}

        LPWSTR lpDriverName = (LPWSTR)Irp->AssociatedIrp.SystemBuffer;
        if (!lpDriverName)
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

		PHOOKED_DRIVER pHookedDriver;
		Status = GetHookedDriverByName(lpDriverName, &pHookedDriver);
		if (!NT_SUCCESS(Status))
		{
			break;
		}

		ULONG OutputBufferLen = Stack->Parameters.DeviceIoControl.OutputBufferLength;
		if (OutputBufferLen < sizeof(HOOKED_DRIVER_INFO))
		{
			Status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		Status = GetDriverInfo(pHookedDriver, lpDriverInfo, pdwDataWritten);
	}
	while (0);

	return Status;
}



/*++

--*/
NTSTATUS HandleIoGetNumberOfHookedDrivers(_In_ PIRP Irp, _Inout_ PIO_STACK_LOCATION Stack, _Out_ PULONG pdwDataWritten)
{
	UNREFERENCED_PARAMETER(Irp);
	PAGED_CODE();

	ULONG OutputBufferLen = Stack->Parameters.DeviceIoControl.OutputBufferLength;

	if (OutputBufferLen < sizeof(UINT32))
		return STATUS_BUFFER_TOO_SMALL;

	UINT32* u32Res = (UINT32*)Irp->AssociatedIrp.SystemBuffer;
	*u32Res = GetNumberOfHookedDrivers();
	*pdwDataWritten = sizeof(UINT32);
	return STATUS_SUCCESS;
}


/*++

--*/
NTSTATUS HandleIoGetNamesOfHookedDrivers(_In_ PIRP Irp, _Inout_ PIO_STACK_LOCATION Stack, _Out_ PULONG pdwDataWritten)
{
	UNREFERENCED_PARAMETER(Irp);
	PAGED_CODE();

	*pdwDataWritten = 0;

	ULONG OutputBufferLen = Stack->Parameters.DeviceIoControl.OutputBufferLength;
	PVOID OutputBuffer = (UINT32*)Irp->AssociatedIrp.SystemBuffer;

	if(!OutputBuffer || OutputBufferLen==0)
		return STATUS_BUFFER_TOO_SMALL;

	NTSTATUS Status = GetNamesOfHookedDrivers(ENABLED_DRIVERS_ONLY, OutputBuffer, OutputBufferLen, pdwDataWritten);
	if (!NT_SUCCESS(Status))
		return Status;

	return STATUS_SUCCESS;
}