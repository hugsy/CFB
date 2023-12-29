#pragma once

#include <wil/resource.h>

#include "Error.hpp"
#include "States.hpp"

namespace CFB::Broker
{
class ManagerBase
{
public:
    ///
    /// @brief Construct a new Manager Base object
    ///
    ///
    ManagerBase();

    ///
    /// @brief Destroy the Manager Base object
    ///
    ///
    ~ManagerBase();

    ///
    /// @brief Synchronizes on the Global state semaphore to execute code only when in a specific state
    ///
    /// @param NewState
    /// @return true
    /// @return false
    ///
    bool
    WaitForState(CFB::Broker::State WantedState);

    ///
    /// @brief Simple wrapper of `Globals.NotifyNewState`
    ///
    /// @param NewState
    /// @return true
    /// @return false
    ///
    bool
    SetState(CFB::Broker::State NewState);

    ///
    /// @brief
    ///
    /// @return true
    /// @return false
    ///
    bool
    NotifyStateChange();

    ///
    /// @brief
    ///
    /// @return true
    /// @return false
    ///
    bool
    NotifyTermination();

    ///
    /// @brief
    ///
    ///
    virtual Result<bool>
    Setup() = 0;


    ///
    /// @brief
    ///
    ///
    virtual void
    Run() = 0;

    ///
    /// @brief
    ///
    /// @return std::string const&
    ///
    virtual std::string const
    Name() = 0;


protected:
    ///
    /// @brief
    ///
    ///
    wil::unique_handle m_hChangedStateEvent;

    ///
    /// @brief
    ///
    ///
    wil::unique_handle m_hTerminationEvent;

    ///
    /// @brief
    ///
    ///
    bool m_bIsShuttingDown;
};


} // namespace CFB::Broker
