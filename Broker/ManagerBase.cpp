#include "ManagerBase.hpp"

#include <thread>

#include "Context.hpp"
#include "Log.hpp"

namespace CFB::Broker
{

ManagerBase::ManagerBase()
{
    m_hChangedStateEvent = wil::unique_handle(::CreateEventW(nullptr, false, false, nullptr));
}

ManagerBase::~ManagerBase()
{
}


bool
ManagerBase::WaitForState(CFB::Broker::State WantedState)
{
    xdbg("Waiting for state '%s' (current '%s')", Utils::ToString(WantedState), Utils::ToString(Globals.State()));

    while ( true )
    {
        ::WaitForSingleObject(m_hChangedStateEvent.get(), INFINITE);

        if ( State() == WantedState )
        {
            return true;
        }

        if ( State() > WantedState )
        {
            return false;
        }
    }

    return false;
}


bool
ManagerBase::NotifyNewState(CFB::Broker::State NewState)
{
    auto bRes = Globals.SetState(NewState);
    if ( bRes )
    {
        ::SetEvent(m_hChangedStateEvent.get());
    }
    return bRes;
}

} // namespace CFB::Broker
