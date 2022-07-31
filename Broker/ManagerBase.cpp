#include "ManagerBase.hpp"

#include "Context.hpp"


namespace CFB::Broker
{


bool
ManagerBase::WaitForState(State WantedState)
{
    while ( true ) // TODO: add better state notif (e.g. shutdown case)
    {
        Globals.StateChangeFlag().wait(false);

        if ( Globals.State() == WantedState )
            return true;
    }

    return false;
}
} // namespace CFB::Broker
