#pragma once

// clang-format off
#include "Common.hpp"

#include <mutex>
// clang-format on


namespace CFB::Broker
{
class ConnectorManager
{
public:
    ConnectorManager();

    ~ConnectorManager();


private:
    ///
    /// @brief This mutex protects state changes
    ///
    std::mutex m_Mutex;

    ///
    /// @brief Changed state notification event.
    ///
    HANDLE m_StateChangedEvent;
};
} // namespace CFB::Broker
