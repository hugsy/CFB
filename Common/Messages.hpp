#pragma once

#include "Common.hpp"

#ifndef CFB_KERNEL_DRIVER
///
/// This file defines the communication protocol and structures for Broker <-> Clients
///

#include <nlohmann/json.hpp>
using json = nlohmann::json;

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

    // Command IDs for Broker
    EnumerateDriverObject = 0x11,
    EnumerateDeviceObject = 0x12,
    GetPendingIrpNumber   = 0x13,
    GetPendingIrp         = 0x14,
};


struct DriverRequest
{
    ///
    /// @brief Mandatory request type id
    ///
    RequestId Id = RequestId::InvalidId;

    ///
    /// @brief Driver name as a wstring, used for
    /// - HookDriver
    /// - UnhookDriver
    /// - EnableMonitoring
    /// - DisableMonitoring
    ///
    std::wstring DriverName;

    ///
    ///@brief used for
    /// - GetPendingIrp
    ///
    u16 NumberOfIrp;
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

std::string
ToString(CFB::Comms::RequestId id);


//
// JSON Converters
//

///
/// @brief DriverRequest -> JSON
///
/// @param dst
/// @param src
///
void
to_json(json& dst, const DriverRequest& src);

///
/// @brief JSON -> DriverRequest
///
/// @param src
/// @param dst
///
void
from_json(const json& src, DriverRequest& dst);

} // namespace CFB::Comms

#endif // CFB_KERNEL_DRIVER
