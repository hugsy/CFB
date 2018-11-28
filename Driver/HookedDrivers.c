#include "HookedDrivers.h"

static LIST_ENTRY HookedDriversHead;
PLIST_ENTRY g_HookedDriversHead = &HookedDriversHead;


/*++

Return the number of the hooked drivers

--*/
UINT32 GetNumberOfHookedDrivers()
{
    if (IsListEmpty(g_HookedDriversHead))
        return 0;

	UINT32 i;
    PLIST_ENTRY Entry;

    for (i = 0, Entry = g_HookedDriversHead->Flink; Entry != g_HookedDriversHead; Entry = Entry->Flink, i++);
	return i;
}



/*++

Determines whether a specific Driver Object is already in the hooked driver list.

--*/
BOOLEAN IsDriverHooked(PDRIVER_OBJECT pDO)
{
    if (IsListEmpty(g_HookedDriversHead))
    {
        return FALSE;
    }

    PLIST_ENTRY Entry = g_HookedDriversHead->Flink;
	
    do
    {
        PHOOKED_DRIVER CurDrv = CONTAINING_RECORD(Entry, HOOKED_DRIVER, ListEntry);

        if (CurDrv->DriverObject == pDO)
        {
            return TRUE;
        }

        Entry = Entry->Flink;

    } while (Entry != g_HookedDriversHead);

	return FALSE;
}


/*++

--*/
PHOOKED_DRIVER GetHookedDriverByName(LPWSTR lpDriverName)
{
    if (IsListEmpty(g_HookedDriversHead))
    {
        return NULL;
    }

    PLIST_ENTRY Entry = g_HookedDriversHead->Flink;

    do
    {
        PHOOKED_DRIVER CurDrv = CONTAINING_RECORD(Entry, HOOKED_DRIVER, ListEntry);
        
        if (wcscmp(CurDrv->Name, lpDriverName) == 0)
        {
            return CurDrv;
        }
            
        Entry = Entry->Flink;

    } while (Entry != g_HookedDriversHead);

    return NULL;
}


