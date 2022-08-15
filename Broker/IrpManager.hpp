#pragma once

// clang-format off
#include "Common.hpp"
#include "ManagerBase.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>
#include <functional>
#include <thread>
#include <vector>

#include <wil/resource.h>
// clang-format on

namespace CFB::Broker
{

class IrpManager : public ManagerBase
{
public:
    ///
    /// @brief Construct a new IRP Manager object
    ///
    IrpManager();

    ///
    /// @brief Destroy the IRP Manager object
    ///
    ~IrpManager();

    ///
    /// @brief
    ///
    /// @return std::string const
    ///
    std::string const
    Name();

    ///
    /// @brief
    ///
    void
    Run();

    ///
    /// @brief
    ///
    /// @return Result<bool>
    ///
    Result<bool>
    Setup();

private:
    ///
    /// @brief Pop the next IRP from the IrpMonitor and push it to the local queue `m_Irps`
    ///
    /// @return usize
    ///
    std::vector<int>
    GetNextIrps();

    ///
    /// @brief Handle to the device
    ///
    wil::unique_handle m_hDevice;

    ///
    /// @brief Handle to the notification event
    ///
    wil::unique_handle m_hNewIrpEvent;

    ///
    /// @brief
    ///
    std::vector<std::function<bool(const int&)>> m_Callbacks;
};

} // namespace CFB::Broker
