#pragma once

// clang-format off
#include "Common.hpp"

#include "json.hpp"
using json = nlohmann::json;
// clang-format on


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
    ///
    /// @brief Mandatory request type id
    ///
    ///
    RequestId Id = RequestId::InvalidId;

    ///
    /// @brief Driver name as a wstring, used for
    /// - HookDriver
    /// - UnhookDriver
    /// - EnableDriver
    /// - DisableDriver
    ///
    std::wstring DriverName;
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
/// @brief JOSN -> DriverRequest
///
/// @param src
/// @param dst
///
void
from_json(const json& src, DriverRequest& dst);

} // namespace CFB::Comms
