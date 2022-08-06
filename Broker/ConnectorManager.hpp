#pragma once

// clang-format off
#include "Common.hpp"
#include "ManagerBase.hpp"
// clang-format on


namespace CFB::Broker
{
class ConnectorManager : ManagerBase
{
public:
    ConnectorManager();

    ~ConnectorManager();

    std::string const
    Name();

    void
    Run();
};
} // namespace CFB::Broker
