#include "IoEnableDisableMonitoring.h"


/*++

--*/
NTSTATUS HandleIoEnableMonitoring( PIRP Irp, PIO_STACK_LOCATION Stack )
{
	UNREFERENCED_PARAMETER( Irp );
	UNREFERENCED_PARAMETER( Stack );
	PAGED_CODE();

	EnableMonitoring();
	CfbDbgPrintOk( L"Monitoring is ENABLED\n" );

	return STATUS_SUCCESS;
}


/*++

--*/
NTSTATUS HandleIoDisableMonitoring( PIRP Irp, PIO_STACK_LOCATION Stack )
{
	UNREFERENCED_PARAMETER( Irp );
	UNREFERENCED_PARAMETER( Stack );
	PAGED_CODE();

	DisableMonitoring();
	CfbDbgPrintOk( L"Monitoring is DISABLED\n" );

	return STATUS_SUCCESS;
}