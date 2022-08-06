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
        xdbg("Waiting for state '%s' (current '%s')", Utils::ToString(WantedState), Utils::ToString(Globals.State()));
        Globals.StateChangeFlag().wait(false);

        if ( Globals.State() == WantedState )
        {
            xdbg("Entered state '%s'", Utils::ToString(WantedState));
            return true;
        }

        if ( Globals.State() > WantedState )
        {
            xdbg("Skipping '%s'", Utils::ToString(WantedState));
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
