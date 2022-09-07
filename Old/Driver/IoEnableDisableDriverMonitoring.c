#include "IoEnableDisableDriverMonitoring.h"


/*++

Update the hooked driver `Enabled` flag, indicating whether this driver specifically should be 
monitored.

--*/
static NTSTATUS SetDriverMonitoringStatus(_In_ PIRP Irp, BOOLEAN NewStatus)
{
	NTSTATUS Status = STATUS_SUCCESS;

	LPWSTR lpDriverName = (LPWSTR)Irp->AssociatedIrp.SystemBuffer;
	if (!lpDriverName)
		return STATUS_INVALID_PARAMETER;


	PHOOKED_DRIVER pHookedDriver;
	Status = GetHookedDriverByName(lpDriverName, &pHookedDriver);
	if (!NT_SUCCESS(Status))
		return Status;

	pHookedDriver->Enabled = !!NewStatus;

	return Status;
}



/*++

Enable driver monitoring IO request.

--*/
NTSTATUS HandleIoEnableDriverMonitoring(_In_ PIRP Irp, _In_ PIO_STACK_LOCATION Stack)
{
	UNREFERENCED_PARAMETER(Stack);
	PAGED_CODE();

	NTSTATUS Status = SetDriverMonitoringStatus(Irp, TRUE);
	if (NT_SUCCESS(Status))
		CfbDbgPrintOk(L"Driver monitoring is ENABLED\n");

	return STATUS_SUCCESS;
}


/*++

Disable driver monitoring IO request.

--*/
NTSTATUS HandleIoDisableDriverMonitoring(_In_ PIRP Irp, _In_ PIO_STACK_LOCATION Stack)
{
	UNREFERENCED_PARAMETER(Stack);
	PAGED_CODE();

	NTSTATUS Status = SetDriverMonitoringStatus(Irp, FALSE);
	if(NT_SUCCESS(Status))
		CfbDbgPrintOk(L"Driver monitoring is DISABLED\n");

	return Status;
}