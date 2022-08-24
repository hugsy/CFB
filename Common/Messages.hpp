#pragma once

#include "Common.hpp"

namespace CFB::Comms
{

enum class RequestId : uptr
{
    InvalidId = 0x00,

    // Command IDs for driver requests
    HookDriver         = 0x01,
    UnhookDriver       = 0x02,
    GetNumberOfDrivers = 0x03,
    GetNamesOfDrivers  = 0x04,
    GetDriverInfo      = 0x05,
    SetEventPointer    = 0x06,
    EnableMonitoring   = 0x07,
    DisableMonitoring  = 0x08,
    StoreTestCase      = 0x09,
    EnableDriver       = 0x0a,
    DisableDriver      = 0x0b,

    // Command IDs for Broker
    EnumerateDriverObject = 0x11,
    EnumerateDeviceObject = 0x12,
};


struct DriverRequest
{
    RequestId Id = RequestId::InvalidId;

    union
    {
        wchar_t DriverName[CFB_DRIVER_MAX_PATH];
        HANDLE IrpNotificationEventHandle;
    } Data = {0};
};


struct DriverResponse
{
    struct
    {
        u32 Status;
        usize DataLength;
    } Header;

    struct
    {
        u8 Data[1];
    } Body;
};

} // namespace CFB::Comms
