#pragma once

#include "Context.hpp"

namespace CFB::Broker
{
class ManagerBase
{
public:
    ///
    /// @brief Synchronizes on the Global state semaphore to execute code only when in a specific state
    ///
    /// @param NewState
    /// @return true
    /// @return false
    ///
    bool
    WaitForState(State NewState);
};


} // namespace CFB::Broker
