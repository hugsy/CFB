#include <Windows.h>


#include "stdafx.h"

#include "../Common/common.h"
#include "../Driver/ioctls.h"

#include "device.h"


/**
 *
 */
HANDLE OpenDevice()
{
	HANDLE hDevice = INVALID_HANDLE_VALUE;

	hDevice = CreateFileW(CFB_DEVICE_NAME,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

	return hDevice;
}


/**
 *
 */
BOOLEAN CloseDevice(HANDLE hDevice)
{
	BOOL bRes = CloseHandle(hDevice);
	return bRes;
}


/**
 *
 */
BOOLEAN QueryDevice(HANDLE hDevice)
{
	BOOLEAN bResult;
	DWORD dwNbDriversHooked = 0, dwBytesReturned = 0;

	bResult = DeviceIoControl(hDevice,
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
