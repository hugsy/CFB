#include "IoEnableDisableMonitoring.h"


BOOLEAN g_IsMonitoringEnabled;
static FAST_MUTEX g_InterceptFastMutex;


/*++

--*/
void InitializeMonitoringStructures()
{
    ExInitializeFastMutex(&g_InterceptFastMutex);

    return;
}


/*++

--*/
static inline NTSTATUS ChangeMonitoringStatus(BOOLEAN NewStatus)
{
    ExAcquireFastMutex(&g_InterceptFastMutex);
    g_IsMonitoringEnabled = NewStatus;
    ExReleaseFastMutex(&g_InterceptFastMutex);

    return STATUS_SUCCESS;
}


/*++

--*/
NTSTATUS EnableMonitoring()
{
    return ChangeMonitoringStatus(TRUE);
}


/*++

--*/
NTSTATUS DisableMonitoring()
{
    return ChangeMonitoringStatus(FALSE);
}


/*++

--*/
BOOLEAN IsMonitoringEnabled()
{
    return g_IsMonitoringEnabled != 0;
}


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