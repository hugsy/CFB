#include "IoGetDriverInfo.h"



/*++


--*/
NTSTATUS GetDriverInfo(UINT32 dwIndex, PHOOKED_DRIVER_INFO pDrvInfo)
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
		CfbDbgPrintErr(L"[-] Exception Code: 0x%X\n", status);
	}

	return status;
}



/*++

--*/
NTSTATUS HandleIoGetDriverInfo(PIRP Irp, PIO_STACK_LOCATION Stack)
{
	UNREFERENCED_PARAMETER(Irp);
	PAGED_CODE();

	NTSTATUS Status = STATUS_SUCCESS;

	do
	{
		ULONG InputBufferLen = Stack->Parameters.DeviceIoControl.InputBufferLength;

		if (InputBufferLen > sizeof(UINT32))
		{
			Status = STATUS_BUFFER_OVERFLOW;
			break;
		}

		ULONG OutputBufferLen = Stack->Parameters.DeviceIoControl.OutputBufferLength;
		if (OutputBufferLen < sizeof(HOOKED_DRIVER_INFO))
		{
			Status = STATUS_BUFFER_TOO_SMALL;
			break;
		}

		UINT32* pdwDriverIndex = (UINT32*)Irp->AssociatedIrp.SystemBuffer;

		Status = GetDriverInfo(*pdwDriverIndex, (PHOOKED_DRIVER_INFO)Irp->AssociatedIrp.SystemBuffer);

	}
	while (0);

	return Status;
}


/*++

--*/
NTSTATUS HandleIoGetNumberOfHookedDrivers(PIRP Irp, PIO_STACK_LOCATION Stack)
{
	UNREFERENCED_PARAMETER(Irp);
	PAGED_CODE();

	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	ULONG OutputBufferLen = Stack->Parameters.DeviceIoControl.OutputBufferLength;

	if (OutputBufferLen < sizeof(UINT32))
	{
		Status = STATUS_BUFFER_TOO_SMALL;
		return Status;
	}

	UINT32* u32Res = (UINT32*)Irp->AssociatedIrp.SystemBuffer;
	*u32Res = GetNumberOfHookedDrivers();
	Status = STATUS_SUCCESS;

	return Status;
}