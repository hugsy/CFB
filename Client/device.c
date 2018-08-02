#include <Windows.h>
#include <wchar.h>

#include "stdafx.h"

#include "../Common/common.h"
#include "../Driver/ioctls.h"



 HANDLE g_hDevice = INVALID_HANDLE_VALUE;


/**
 *
 */
BOOLEAN OpenCfbDevice()
{
	g_hDevice = CreateFileW(CFB_USER_DEVICE_NAME,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);
	
	return g_hDevice != INVALID_HANDLE_VALUE;
}


/**
 *
 */
BOOLEAN CloseCfbDevice()
{
	BOOLEAN bRes = CloseHandle(g_hDevice);
	return bRes;
}


/**
 *
 */
BOOLEAN EnumerateHookedDrivers()
{
	BOOLEAN bResult;
	DWORD dwNbDriversHooked = 0, dwBytesReturned = 0;

	bResult = DeviceIoControl(g_hDevice,
		IOCTL_GetNumberOfDrivers,
		NULL,
		0,
		&dwNbDriversHooked,
		sizeof(DWORD),
		&dwBytesReturned,
		(LPOVERLAPPED)NULL);

	if (bResult == FALSE)
	{
		wprintf(L"DeviceIoControl(IOCTL_GetNumberOfDrivers) failed\n");
		return FALSE;
	}

	return TRUE;
}


/**
 *
 */
BOOLEAN HookDriver(LPWSTR lpDriver)
{
	DWORD dwBytesReturned;
	DWORD dwDriverLen = (DWORD)(wcslen(lpDriver) * sizeof(WCHAR));

	BOOLEAN bResult = DeviceIoControl(g_hDevice,
		IOCTL_AddDriver,
		lpDriver,
		dwDriverLen,
		NULL,
		0,
		&dwBytesReturned,
		(LPOVERLAPPED)NULL);

	return bResult;
}



/**
 *
 */
NTSTATUS UnhookDriver(LPWSTR lpDriver)
{
	DWORD dwBytesReturned;
	DWORD dwDriverLen = (DWORD)(wcslen(lpDriver) * sizeof(WCHAR));

	BOOLEAN bResult = DeviceIoControl(g_hDevice,
		IOCTL_RemoveDriver,
		lpDriver,
		dwDriverLen,
		NULL,
		0,
		&dwBytesReturned,
		(LPOVERLAPPED)NULL);

	return bResult;
}