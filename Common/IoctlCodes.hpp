#pragma once

#ifndef CTL_CODE
#define CTL_CODE(DeviceType, Function, Method, Access)                                                                 \
    (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif // CTL_CODE


namespace CFB::Comms
{

///
///@brief An enumeration class of the available IOCTL supported by the driver
///
enum class Ioctl : u32
{
    // clang-format off

    ///
    ///@brief ControlDriver - obsolete
    ///
    ControlDriver      = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief HookDriver
    ///
    HookDriver         = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief UnhookDriver
    ///
    UnhookDriver       = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief GetNumberOfDrivers
    ///
    GetNumberOfDrivers = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief GetNamesOfDrivers
    ///
    GetNamesOfDrivers  = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief GetDriverInfo
    ///
    GetDriverInfo      = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief SetEventPointer
    ///
    SetEventPointer    = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief EnableMonitoring
    ///
    EnableMonitoring   = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief DisableMonitoring
    ///
    DisableMonitoring  = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief StoreTestCase
    ///
    StoreTestCase      = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief EnableDriver
    ///
    EnableDriver       = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80a, METHOD_BUFFERED, FILE_ANY_ACCESS),

    ///
    ///@brief DisableDriver
    ///
    DisableDriver      = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80b, METHOD_BUFFERED, FILE_ANY_ACCESS),


    // clang-format on
};

} // namespace CFB::Comms
