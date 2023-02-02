#pragma once

// clang-format off
#include "Common.hpp"
#include "ManagerBase.hpp"

#include "Connectors/Base.hpp"
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
    ConnectorManager()
    {
    }

    ///
    /// @brief Destroy the Connector Manager object
    ///
    ///
    ~ConnectorManager()
    {
    }

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

    ///
    ///@brief Get the Connector By Name object
    ///
    ///@param ConnectorName
    ///@return Result<std::shared_ptr<Connectors::ConnectorBase>>
    ///
    Result<std::shared_ptr<Connectors::ConnectorBase>>
    GetConnectorByName(std::string_view const& ConnectorName);

private:
};
} // namespace CFB::Broker
