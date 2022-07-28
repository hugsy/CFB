#pragma once

#ifndef CTL_CODE
#define CTL_CODE(DeviceType, Function, Method, Access)                                                                 \
    (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif // CTL_CODE

// clang-format off
#define _IOCTL(Function)             CTL_CODE(FILE_DEVICE_UNKNOWN, Function, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HookDriver			_IOCTL(0x801)
#define IOCTL_UnhookDriver			_IOCTL(0x802)
#define IOCTL_GetNumberOfDrivers	_IOCTL(0x803)
#define IOCTL_GetNamesOfDrivers		_IOCTL(0x804)
#define IOCTL_GetDriverInfo			_IOCTL(0x805)
#define IOCTL_SetEventPointer       _IOCTL(0x806)
#define IOCTL_EnableMonitoring		_IOCTL(0x807)
#define IOCTL_DisableMonitoring		_IOCTL(0x808)
#define IOCTL_StoreTestCase 		_IOCTL(0x809)
#define IOCTL_EnableDriver	 		_IOCTL(0x80a)
#define IOCTL_DisableDriver	 		_IOCTL(0x80b)

// clang-format on
