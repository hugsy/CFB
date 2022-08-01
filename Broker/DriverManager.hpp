#pragma once

// clang-format off
#include "Common.hpp"
#include "Broker.hpp"
#include "ManagerBase.hpp"

#include <wil/resource.h>
// clang-format on

namespace CFB::Broker
{
class DriverManager : ManagerBase
{
public:
    ///
    /// @brief Construct a new Driver Manager object
    ///
    ///
    DriverManager();

    ///
    /// @brief Destroy the Driver Manager object
    ///
    ///
    ~DriverManager();

    ///
    /// @brief
    ///
    ///
    void
    Run();

    ///
    /// @brief Takes a request Task, and creates and send a valid DeviceIoControl() to the IrpDumper driver. The
    /// function also builds a response Task from the response of the DeviceIoControl().
    ///
    /// @return Result<u32>
    ///
    Result<u32>
    SendIoctl();

private:
    ///
    /// @brief Handle to the device
    ///
    ///
    wil::unique_handle m_hDevice;
};

} // namespace CFB::Broker
