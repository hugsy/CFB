#pragma once

#ifndef CTL_CODE
#define CTL_CODE(DeviceType, Function, Method, Access)                                                                 \
    (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif // CTL_CODE

// clang-format off
#define IOCTL_ControlDriver			  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
// clang-format on
