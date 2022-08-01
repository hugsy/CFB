#include "ManagerBase.hpp"

#include <thread>

#include "Context.hpp"
#include "Log.hpp"

namespace CFB::Broker
{

bool
ManagerBase::WaitForState(CFB::Broker::State WantedState)
{
    // TODO: add a check for shutdown state
    while ( true )
    {
        dbg("[TID=%x] Waiting for state '%s' (current '%s')",
            std::this_thread::get_id(),
            Utils::ToString(WantedState),
            CFB::Broker::Utils::ToString(Globals.State()));
        Globals.StateChangeFlag().wait(false);

        if ( Globals.State() == WantedState )
        {
            dbg("[TID=%x] Entered state '%s'", std::this_thread::get_id(), Utils::ToString(WantedState));
            return true;
        }

        if ( Globals.State() > WantedState )
        {
            dbg("[TID=%x] Skipping '%s'", std::this_thread::get_id(), Utils::ToString(WantedState));
            return false;
        }
    }

    return false;
}

bool
ManagerBase::NotifyNewState(CFB::Broker::State NewState)
{
    return Globals.NotifyNewState(NewState);
}

} // namespace CFB::Broker
