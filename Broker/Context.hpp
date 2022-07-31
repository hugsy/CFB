#pragma once

#include <atomic>
#include <mutex>

#include "ServiceManager.hpp"


namespace CFB::Broker
{
enum class State : int
{
    Uninitialized = 1,
    ServiceManagerReady,
    DriverManagerReady,
    ConnectorManagerReady,
};
} // namespace CFB::Broker


class GlobalContext
{
public:
    GlobalContext();


    ///
    /// @brief Set new state
    ///
    /// @return true
    /// @return false
    ///
    bool NotifyNewState(CFB::Broker::State);

    std::atomic_flag const&
    StateChangeFlag() const;

    CFB::Broker::State const
    State() const;

    CFB::Broker::ServiceManager ServiceManager;

private:
    ///
    /// @brief This mutex protects state changes
    ///
    std::mutex m_Mutex;

    ///
    /// @brief The manager current state
    ///
    CFB::Broker::State m_State;

    std::atomic_flag m_AtomicStateChangeFlag;
};


extern GlobalContext Globals;
