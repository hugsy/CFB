#include "HookedDrivers.h"


/*++

--*/
UINT32 GetNumberOfHookedDrivers()
{
	UINT32 i;
	PHOOKED_DRIVER ptr;
	for (i = 0, ptr = g_HookedDriversHead; ptr; ptr = ptr->Next, i++);
	return i;
}


/*++

--*/
PHOOKED_DRIVER GetLastHookedDriver()
{
	PHOOKED_DRIVER lastDriver;
	for (lastDriver = g_HookedDriversHead; lastDriver; lastDriver = lastDriver->Next) {}
	return lastDriver;
}


/*++

--*/
PHOOKED_DRIVER GetPreviousHookedDriver(PHOOKED_DRIVER pDriver)
{
	if (!g_HookedDriversHead)
		return NULL;

	PHOOKED_DRIVER pCurDriver = g_HookedDriversHead->Next;
	PHOOKED_DRIVER pPrevDriver = g_HookedDriversHead;
	BOOLEAN Found = FALSE;

	while (pCurDriver)
	{
		if (pCurDriver == pDriver)
		{
			Found = TRUE;
			break;
		}

		pPrevDriver = pCurDriver;
		pCurDriver = pCurDriver->Next;
	}

	//
	// if not found
	//
	if (!Found)
		return NULL;

	return pPrevDriver;
}


/*++

--*/
PHOOKED_DRIVER GetNextHookedDriver(PHOOKED_DRIVER pDriver)
{
	if (!pDriver)
		return NULL;

	return pDriver->Next;
}


/*++

--*/
BOOLEAN IsDriverHooked(PDRIVER_OBJECT pObj)
{
	PHOOKED_DRIVER ptr;
	for (ptr = g_HookedDriversHead; ptr; ptr = ptr->Next)
	{
		if (ptr->DriverObject == pObj)
		{
			return TRUE;
		}
	}
	return FALSE;
}


/*++

--*/
PHOOKED_DRIVER GetHookedDriverByName(LPWSTR lpDriverName)
{
	PHOOKED_DRIVER Driver = g_HookedDriversHead;

	while (Driver)
	{
		if (wcscmp(Driver->Name, lpDriverName) == 0)
			return Driver;

		Driver = Driver->Next;
	}

	return NULL;
}


/*++

--*/
PHOOKED_DRIVER GetHookedDriverByIndex(UINT32 dwIndex)
{
	if (dwIndex >= GetNumberOfHookedDrivers())
	{
		CfbDbgPrint(L"Cannot reach index %ld\n", dwIndex);
		return NULL;
	}

	PHOOKED_DRIVER Driver = g_HookedDriversHead;

	for (; dwIndex; dwIndex--, Driver = Driver->Next);

	return Driver;
}