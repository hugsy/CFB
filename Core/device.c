#include <Windows.h>
#include <wchar.h>

#include "stdafx.h"

#include "../Common/common.h"
#include "../Driver/IoctlCodes.h"




static HANDLE g_hDevice = INVALID_HANDLE_VALUE;


/*++

Get a R/W handle to the CFB device

--*/
BOOL OpenCfbDevice()
{
	g_hDevice = CreateFileW(CFB_USER_DEVICE_NAME,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	return g_hDevice != INVALID_HANDLE_VALUE;
}


/*++

Close the handle to the CFB device

--*/
BOOL CloseCfbDevice()
{
	BOOL bRes = CloseHandle(g_hDevice);
	if(bRes)
		g_hDevice = INVALID_HANDLE_VALUE;
	return bRes;
}


/*++

Read new IRP data from the device.

--*/
__declspec(dllexport) BOOL ReadCfbDevice(LPVOID Buffer, DWORD BufSize, LPDWORD lpNbBytesRead)
{
	BOOL bRes = ReadFile(g_hDevice, Buffer, BufSize, lpNbBytesRead,	NULL );	
	return bRes;
}


/*++

--*/
__declspec(dllexport) DWORD GetCfbMessageHeaderSize()
{
	return sizeof( SNIFFED_DATA_HEADER );
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
BOOL GetHookedDriverInfo(DWORD dwDriverIndex, PHOOKED_DRIVER_INFO hDrvInfo)
{
	BOOL bResult;
	DWORD dwNbDriversHooked = 0, dwBytesReturned = 0;

	bResult = DeviceIoControl(g_hDevice,
		IOCTL_GetDriverInfo,
		&dwDriverIndex,
		sizeof(DWORD),
		&hDrvInfo,
		sizeof(HOOKED_DRIVER_INFO),
		&dwBytesReturned,
		(LPOVERLAPPED)NULL);

	return bResult;
}


/*++

Send the IO request to add a driver to the hooked list.

--*/
__declspec(dllexport) BOOL HookDriver(LPWSTR lpDriverName)
{
	DWORD dwBytesReturned;
	DWORD dwDriverNameLen = (DWORD)(wcslen( lpDriverName ) * sizeof(WCHAR))+2;

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
__declspec(dllexport) BOOL UnhookDriver(LPWSTR lpDriverName )
{
	DWORD dwBytesReturned;
	DWORD dwDriverNameLen= (DWORD)(wcslen( lpDriverName ) * sizeof(WCHAR))+2;

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