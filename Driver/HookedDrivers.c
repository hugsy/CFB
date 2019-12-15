#include "HookedDrivers.h"

static LIST_ENTRY HookedDriversHead;
PLIST_ENTRY g_HookedDriverHead = &HookedDriversHead;

static KSPIN_LOCK HookedDriverSpinLock;
static KLOCK_QUEUE_HANDLE HookedDriverSpinLockQueue;

static FAST_MUTEX DriverListMutex;


/*++

Routine Description:

Initialize the structures used as part of the driver hooking.


Arguments:


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
void InitializeHookedDriverStructures()
{
    KeInitializeSpinLock(&HookedDriverSpinLock);
	ExInitializeFastMutex(&DriverListMutex);
    return;
}


/*++

Routine Description:

Return the number of the hooked drivers


Arguments:


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
UINT32 GetNumberOfHookedDrivers()
{
    UINT32 i = 0;
    

    KeAcquireInStackQueuedSpinLock(&HookedDriverSpinLock, &HookedDriverSpinLockQueue);

    if (!IsListEmpty(g_HookedDriverHead))
    {
		PLIST_ENTRY Entry;
        for (i = 0, Entry = g_HookedDriverHead->Flink; 
            Entry != g_HookedDriverHead; 
            Entry = Entry->Flink, i++);
    }

    KeReleaseInStackQueuedSpinLock(&HookedDriverSpinLockQueue);

	return i;
}



/*++

Routine Description:

Determines whether a specific Driver Object is already in the hooked driver list.


Arguments:


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
BOOLEAN IsDriverHooked(_In_ PDRIVER_OBJECT pDriverObject)
{
    BOOLEAN bRes = FALSE;

    KeAcquireInStackQueuedSpinLock(&HookedDriverSpinLock, &HookedDriverSpinLockQueue);

    if (!IsListEmpty(g_HookedDriverHead))
    {

        PLIST_ENTRY Entry = g_HookedDriverHead->Flink;

        do
        {
            PHOOKED_DRIVER CurDrv = CONTAINING_RECORD(Entry, HOOKED_DRIVER, ListEntry);

            if (CurDrv->DriverObject == pDriverObject)
            {
                bRes = TRUE;
                break;
            }

            Entry = Entry->Flink;

        } 
		while (Entry != g_HookedDriverHead);

    }

    KeReleaseInStackQueuedSpinLock(&HookedDriverSpinLockQueue);

	return bRes;
}


/*++

Routine Description:

Returns a pointer to a HOOKED_DRIVER object if its name is found in the list of hooked
drivers.


Arguments:


Return Value:

	Returns STATUS_SUCCESS on success.

	--*/
NTSTATUS GetHookedDriverByName(_In_ LPWSTR lpDriverName, _Out_ PHOOKED_DRIVER *pHookedDrv)
{
	NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;

	CfbDbgPrintInfo(L"GetHookedDriverByName(lpDriverName='%s')\n", lpDriverName);

    KeAcquireInStackQueuedSpinLock(&HookedDriverSpinLock, &HookedDriverSpinLockQueue);

    if (!IsListEmpty(g_HookedDriverHead))
    {
        PLIST_ENTRY Entry = g_HookedDriverHead->Flink;

        do
        {
            PHOOKED_DRIVER CurDrv = CONTAINING_RECORD(Entry, HOOKED_DRIVER, ListEntry);

            if (_wcsicmp(CurDrv->Name, lpDriverName) == 0)
            {
				*pHookedDrv = CurDrv;
				Status = STATUS_SUCCESS;
                break;
            }

            Entry = Entry->Flink;

        } 
		while (Entry != g_HookedDriverHead);

    }

    KeReleaseInStackQueuedSpinLock(&HookedDriverSpinLockQueue);

    return Status;
}



/*++

Routine Description:

 Find the original function for the driver


Arguments:

	- DeviceObject

Return Value:

	Returns a pointer to the hooked driver on success, NULL otherwise

--*/
PHOOKED_DRIVER GetHookedDriverFromDeviceObject(_In_ PDEVICE_OBJECT DeviceObject)
{
	if (IsListEmpty(g_HookedDriverHead))
		return NULL;

	PHOOKED_DRIVER Driver = NULL;
	PLIST_ENTRY Entry = g_HookedDriverHead->Flink;

	do
	{
		Driver = CONTAINING_RECORD(Entry, HOOKED_DRIVER, ListEntry);

		if (Driver->DriverObject == DeviceObject->DriverObject)
			return Driver;

		Entry = Entry->Flink;

	} 
	while (Entry != g_HookedDriverHead);

	return NULL;
}