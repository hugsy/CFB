#pragma once

#include <atomic>
#include <mutex>

#include "ServiceManager.hpp"
#include "States.hpp"


class GlobalContext
{
public:
    ///
    /// @brief Construct a new Global Context object
    ///
    GlobalContext();

    ///
    /// @brief Set new state
    ///
    /// @return true
    /// @return false
    ///
    bool NotifyNewState(CFB::Broker::State);

    ///
    /// @brief
    ///
    /// @return std::atomic_flag const&
    ///
    std::atomic_flag const&
    StateChangeFlag() const;

    ///
    /// @brief
    ///
    /// @return CFB::Broker::State const
    ///
    CFB::Broker::State const
    State() const;

    ///
    /// @brief
    ///
    ///
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

    ///
    /// @brief
    ///
    ///
    std::atomic_flag m_AtomicStateChangeFlag;
};


extern GlobalContext Globals;
