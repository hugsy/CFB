#include "HookedDrivers.h"

static LIST_ENTRY HookedDriversHead;
PLIST_ENTRY g_HookedDriverHead = &HookedDriversHead;

static KSPIN_LOCK HookedDriverSpinLock;
static KLOCK_QUEUE_HANDLE HookedDriverSpinLockQueue;


/*++

Initialize the structures used as part of the driver hooking.

--*/
void InitializeHookedDriverStructures()
{
    KeInitializeSpinLock(&HookedDriverSpinLock);
    return;
}


/*++

Return the number of the hooked drivers

--*/
UINT32 GetNumberOfHookedDrivers()
{
    UINT32 i = 0;
    PLIST_ENTRY Entry;

    KeAcquireInStackQueuedSpinLock(&HookedDriverSpinLock, &HookedDriverSpinLockQueue);

    if (!IsListEmpty(g_HookedDriverHead))
    {
        for (i = 0, Entry = g_HookedDriverHead->Flink; 
            Entry != g_HookedDriverHead; 
            Entry = Entry->Flink, i++);
    }

    KeReleaseInStackQueuedSpinLock(&HookedDriverSpinLockQueue);

	return i;
}



/*++

Determines whether a specific Driver Object is already in the hooked driver list.

--*/
BOOLEAN IsDriverHooked(IN PDRIVER_OBJECT pDriverObject)
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

Returns a pointer to a HOOKED_DRIVER object if its name is found in the list of hooked
drivers.

--*/
NTSTATUS GetHookedDriverByName(IN LPWSTR lpDriverName, OUT PHOOKED_DRIVER *pHookedDrv)
{
	NTSTATUS Status = STATUS_OBJECT_NAME_NOT_FOUND;


    KeAcquireInStackQueuedSpinLock(&HookedDriverSpinLock, &HookedDriverSpinLockQueue);

    if (!IsListEmpty(g_HookedDriverHead))
    {
        PLIST_ENTRY Entry = g_HookedDriverHead->Flink;

        do
        {
            PHOOKED_DRIVER CurDrv = CONTAINING_RECORD(Entry, HOOKED_DRIVER, ListEntry);
			CfbDbgPrintInfo(L"trying to remove %p (%s)\n", CurDrv, CurDrv->Name);
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


