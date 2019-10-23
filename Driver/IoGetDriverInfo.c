#include "IoGetDriverInfo.h"



/*++


--*/
static NTSTATUS GetDriverInfo(LPWSTR lpDriverName, PHOOKED_DRIVER_INFO pDrvInfo)
{
	
	if (!lpDriverName)
	{
		return STATUS_INVALID_PARAMETER;
	}

	PHOOKED_DRIVER pHookedDriver;

	NTSTATUS Status = GetHookedDriverByName(lpDriverName, &pHookedDriver);

	if (!NT_SUCCESS(Status))
	{
		return Status;
	}


	__try
	{
		RtlSecureZeroMemory(pDrvInfo, sizeof(HOOKED_DRIVER_INFO));
		RtlCopyMemory((PVOID)pDrvInfo->Enabled, (PVOID) pHookedDriver->Enabled, sizeof(BOOLEAN));
		RtlCopyMemory((PVOID)pDrvInfo->Name, (PVOID) pHookedDriver->Name, HOOKED_DRIVER_MAX_NAME_LEN * sizeof(WCHAR));
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
NTSTATUS HandleIoGetDriverInfo(PIRP Irp, PIO_STACK_LOCATION Stack)
{
	UNREFERENCED_PARAMETER(Irp);
	PAGED_CODE();

	NTSTATUS Status = STATUS_SUCCESS;

	do
	{
		ULONG OutputBufferLen = Stack->Parameters.DeviceIoControl.OutputBufferLength;
		if (OutputBufferLen < sizeof(HOOKED_DRIVER_INFO))
		{
			Status = STATUS_BUFFER_TOO_SMALL;
			break;
		}
        PHOOKED_DRIVER_INFO lpDriverInfo = (PHOOKED_DRIVER_INFO)Irp->AssociatedIrp.SystemBuffer;

        LPWSTR lpDriverName = (LPWSTR)Irp->AssociatedIrp.SystemBuffer;

        if (!lpDriverName)
        {
            Status = STATUS_UNSUCCESSFUL;
            break;
        }

		Status = GetDriverInfo(lpDriverName, lpDriverInfo);
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


	ULONG OutputBufferLen = Stack->Parameters.DeviceIoControl.OutputBufferLength;

	if (OutputBufferLen < sizeof(UINT32))
	{
		return STATUS_BUFFER_TOO_SMALL;
	}

	UINT32* u32Res = (UINT32*)Irp->AssociatedIrp.SystemBuffer;
	*u32Res = GetNumberOfHookedDrivers();

	return STATUS_SUCCESS;
}