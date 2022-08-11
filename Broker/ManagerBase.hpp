#pragma once

#include <wil/resource.h>

#include "States.hpp"

#ifdef _DEBUG
#define xdbg(fmt, ...)                                                                                                 \
    dbg("[%s - CID:%d/%d] " fmt, Name().c_str(), ::GetCurrentProcessId(), ::GetCurrentThreadId(), __VA_ARGS__)
#else
#define xdbg(fmt, ...)
#endif // _DEBUG

namespace CFB::Broker
{
class ManagerBase
{
public:
    ManagerBase();

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
    NotifyNewState(CFB::Broker::State NewState);

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


private:
    ///
    /// @brief
    ///
    ///
    wil::unique_handle m_hChangedStateEvent;
};


} // namespace CFB::Broker
