#pragma once

// clang-format off
#include "Common.hpp"
#include "ManagerBase.hpp"
// clang-format on


namespace CFB::Broker
{
class ConnectorManager : public ManagerBase
{
public:
    ///
    /// @brief Construct a new Connector Manager object
    ///
    ///
    ConnectorManager();

    ///
    /// @brief Destroy the Connector Manager object
    ///
    ///
    ~ConnectorManager();

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
    /// @return Result<bool>
    ///
    Result<bool>
    Setup();

    ///
    /// @brief
    ///
    ///
    void
    Run();
};
} // namespace CFB::Broker
