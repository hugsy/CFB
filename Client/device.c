#include <Windows.h>
#include <wchar.h>

#include "stdafx.h"

#include "../Common/common.h"
#include "../Driver/ioctls.h"



static HANDLE g_hDevice = INVALID_HANDLE_VALUE;


/*++

--*/
BOOL OpenCfbDevice()
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


/*++

--*/
BOOL CloseCfbDevice()
{
	BOOL bRes = CloseHandle(g_hDevice);
	g_hDevice = INVALID_HANDLE_VALUE;
	return bRes;
}


/*++

Returns the number of currently hooked drivers in the klist.

--*/
BOOL GetNumberOfDrivers(PDWORD pdwNbDrivers)
{
	BOOL bResult;
	DWORD dwNbDriversHooked = 0, dwBytesReturned = 0;

	bResult = DeviceIoControl(g_hDevice,
		IOCTL_GetNumberOfDrivers,
		NULL,
		0,
		&dwNbDriversHooked,
		sizeof(DWORD),
		&dwBytesReturned,
		(LPOVERLAPPED)NULL);

	*pdwNbDrivers = dwNbDriversHooked;

	return bResult;
}


/*++

Get the information structure about drivers at index provided.

--*/
BOOL GetHookedDriverInfo(DWORD dwDriverIndex)
{
	BOOL bResult;
	DWORD dwNbDriversHooked = 0, dwBytesReturned = 0;

	bResult = DeviceIoControl(g_hDevice,
		IOCTL_GetDriverInfo,
		&dwDriverIndex,
		sizeof(DWORD),
		&dwNbDriversHooked,
		sizeof(DWORD),
		&dwBytesReturned,
		(LPOVERLAPPED)NULL);

	return bResult;
}


/*++

Send the IO request to add a driver to the hooked list.

--*/
BOOL HookDriver(LPWSTR lpDriver)
{
	DWORD dwBytesReturned;
	DWORD dwDriverLen = (DWORD)(wcslen(lpDriver) * sizeof(WCHAR));

	BOOL bResult = DeviceIoControl(g_hDevice,
		IOCTL_AddDriver,
		lpDriver,
		dwDriverLen,
		NULL,
		0,
		&dwBytesReturned,
		(LPOVERLAPPED)NULL);

	return bResult;
}



/*++

Send the IO request to remove a driver to the hooked list.

--*/
BOOL UnhookDriver(LPWSTR lpDriver)
{
	DWORD dwBytesReturned;
	DWORD dwDriverLen = (DWORD)(wcslen(lpDriver) * sizeof(WCHAR));

	BOOL bResult = DeviceIoControl(g_hDevice,
		IOCTL_RemoveDriver,
		lpDriver,
		dwDriverLen,
		NULL,
		0,
		&dwBytesReturned,
		(LPOVERLAPPED)NULL);

	return bResult;
}