#include "ManagerBase.hpp"

#include <thread>

#include "Context.hpp"
#include "Log.hpp"

namespace CFB::Broker
{

bool
ManagerBase::WaitForState(CFB::Broker::State WantedState)
{
    xdbg("Waiting for state '%s' (current '%s')", Utils::ToString(WantedState), Utils::ToString(Globals.State()));
    auto bRes = Globals.WaitForState(WantedState);
    return bRes;
}

bool
ManagerBase::NotifyNewState(CFB::Broker::State NewState)
{
    auto bRes = Globals.NotifyNewState(NewState);
    return bRes;
}

} // namespace CFB::Broker
