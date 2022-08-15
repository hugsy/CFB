#include "ManagerBase.hpp"

#include <thread>

#include "Context.hpp"
#include "Log.hpp"

namespace CFB::Broker
{

ManagerBase::ManagerBase()
{
    {
        wil::unique_handle hEvent(::CreateEventW(nullptr, true, false, nullptr));
        if ( !hEvent )
        {
            CFB::Log::perror("CreateEventW(TerminationEvent)");
            throw std::runtime_error("ManagerBase::CreateEvent() failed, cannot continue");
        }

        m_hChangedStateEvent = std::move(hEvent);
    }
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
        //
        // if the wanted state current or already, leave
        //
        if ( Globals.State() >= WantedState )
        {
            break;
        }

        //
        // Otherwise wait to be signaled
        //

        DWORD dwOut = ::WaitForSingleObject(m_hChangedStateEvent.get(), INFINITE);
        if ( dwOut != WAIT_OBJECT_0 )
        {
            CFB::Log::perror("ManagerBase::WaitForState::WaitForSingleObject");
            return false;
        }
    }

    return true;
}


bool
ManagerBase::SetState(CFB::Broker::State NewState)
{
    xdbg("Notifying state change '%s' -> '%s'", Utils::ToString(Globals.State()), Utils::ToString(NewState));

    auto bRes = Globals.SetState(NewState);
    return bRes;
}


bool
ManagerBase::NotifyStateChange()
{
    auto bRes = ::SetEvent(m_hChangedStateEvent.get());
    if ( false == bRes )
    {
        CFB::Log::perror("ManagerBase::NotifyStateChange::SetEvent");
    }
    return bRes;
}

} // namespace CFB::Broker
