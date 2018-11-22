#include "device.h"

#include "stdafx.h"


static HANDLE g_hDevice = INVALID_HANDLE_VALUE;


/*++

Get a R/W handle to the CFB device

--*/
BOOL OpenCfbDevice()
{
	g_hDevice = CreateFileW(
		CFB_USER_DEVICE_NAME,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	return g_hDevice != INVALID_HANDLE_VALUE;
}


/*++

Close the handle to the CFB device

--*/
BOOL CloseCfbDevice()
{
	BOOL bRes = CloseHandle(g_hDevice);
	if(bRes)
	{
		g_hDevice = INVALID_HANDLE_VALUE;
	}

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
__declspec(dllexport) BOOL HookDriver(IN LPWSTR lpDriverName)
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
__declspec(dllexport) BOOL UnhookDriver(IN LPWSTR lpDriverName )
{
	DWORD dwBytesReturned;
	DWORD dwDriverNameLen = (DWORD)(wcslen( lpDriverName ) * sizeof(WCHAR))+2;

	BOOL bResult = DeviceIoControl(
        g_hDevice,
		IOCTL_RemoveDriver,
		lpDriverName,
		dwDriverNameLen,
		NULL,
		0,
		&dwBytesReturned,
		(LPOVERLAPPED)NULL
    );

	return bResult;
}


/*++

Send the IO request to send shared event handle to the driver to receive notification
of new messages.

--*/
__declspec(dllexport) BOOL SetEventNotificationHandle( IN HANDLE hEvent )
{
	DWORD dwBytesReturned;

	BOOL bResult = DeviceIoControl( 
        g_hDevice,
		IOCTL_SetEventPointer,
		&hEvent,
		sizeof(HANDLE),
		NULL,
		0,
		&dwBytesReturned,
		(LPOVERLAPPED)NULL 
    );

	return bResult;
}


/*++

Enable / Disable the monitoring.

--*/
 static BOOL ChangeMonitoringStatus(IN BOOL Status)
{
	DWORD dwBytesReturned;

	BOOL bResult = DeviceIoControl( 
        g_hDevice,
		Status ? IOCTL_EnableMonitoring : IOCTL_DisableMonitoring,
		NULL,
		0,
		NULL,
		0,
		&dwBytesReturned,
		(LPOVERLAPPED)NULL 
    );

	return bResult;
}

 __declspec(dllexport) BOOL EnableMonitoring()
 {
	 return ChangeMonitoringStatus( 1 /* Monitoring On */ );
 }

 __declspec(dllexport) BOOL DisableMonitoring()
 {
	 return ChangeMonitoringStatus( 0 /* Monitoring Off */);
 }