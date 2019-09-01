#include "device.h"



/*++

Get a handle to the device IrpDumper. Several checks are performed by the driver, so a 
failure to get a handle should be considered critical.

--*/
BOOL OpenDevice()
{
	g_hDevice = CreateFile(
		CFB_USER_DEVICE_NAME,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL
	);

	return g_hDevice != INVALID_HANDLE_VALUE;
}


/*++

--*/
BOOL CloseDevice()
{
	BOOL bRes = CloseHandle(g_hDevice);
	g_hDevice = INVALID_HANDLE_VALUE;
	return bRes;
}


/*++

Returns the number of currently hooked drivers in the klist.

--*/
_Success_(return) BOOL GetNumberOfDrivers(_Out_ PDWORD pdwNbDrivers)
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
_Success_(return) BOOL GetHookedDriverInfo(_In_ DWORD dwDriverIndex, _Out_ PHOOKED_DRIVER_INFO hDrvInfo)
{
	BOOL bResult;
	DWORD dwNbDriversHooked = 0, dwBytesReturned = 0;

	bResult = DeviceIoControl(
		g_hDevice,
		IOCTL_GetDriverInfo,
		&dwDriverIndex,
		sizeof(DWORD),
		hDrvInfo,
		sizeof(HOOKED_DRIVER_INFO),
		&dwBytesReturned,
		(LPOVERLAPPED)NULL
	);

	return bResult;
}


/*++

Send the IO request to add a driver to the hooked list.

--*/
_Success_(return) BOOL HookDriver(_In_ LPWSTR lpDriverName)
{
	DWORD dwBytesReturned;
	DWORD dwDriverNameLen = (DWORD)(wcslen(lpDriverName) * sizeof(WCHAR)) + 2;

	BOOL bResult = DeviceIoControl(g_hDevice,
		IOCTL_AddDriver,
		lpDriverName,
		dwDriverNameLen,
		NULL,
		0,
		&dwBytesReturned,
		(LPOVERLAPPED)NULL);

	return bResult;
}



/*++

Send the IO request to remove a driver to the hooked list.

--*/
_Success_(return) BOOL UnhookDriver(_In_ LPWSTR lpDriverName)
{
	DWORD dwBytesReturned;
	DWORD dwDriverNameLen = (DWORD)(wcslen(lpDriverName) * sizeof(WCHAR)) + 2;

	BOOL bResult = DeviceIoControl(g_hDevice,
		IOCTL_RemoveDriver,
		lpDriverName,
		dwDriverNameLen,
		NULL,
		0,
		&dwBytesReturned,
		(LPOVERLAPPED)NULL);

	return bResult;
}