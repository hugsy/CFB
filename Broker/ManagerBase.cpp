#include "ManagerBase.hpp"

#include <thread>

#include "Context.hpp"
#include "Log.hpp"

namespace CFB::Broker
{

ManagerBase::ManagerBase() : m_bIsShuttingDown(false)
{
    {
        wil::unique_handle hEvent(::CreateEventW(nullptr, true, false, nullptr));
        if ( !hEvent )
        {
            CFB::Log::perror("CreateEventW(StateChanged)");
            throw std::runtime_error("ManagerBase::CreateEvent() failed, cannot continue");
        }

        m_hChangedStateEvent = std::move(hEvent);
    }

    {
        wil::unique_handle hEvent(::CreateEventW(nullptr, true, false, nullptr));
        if ( !hEvent )
        {
            CFB::Log::perror("CreateEventW(TerminationEvent)");
            throw std::runtime_error("ManagerBase::CreateEvent() failed, cannot continue");
        }

        m_hTerminationEvent = std::move(hEvent);
    }
}


ManagerBase::~ManagerBase()
{
}


bool
ManagerBase::WaitForState(CFB::Broker::State WantedState)
{
    const CFB::Broker::State CurrentState = Globals.State();
    xdbg("Waiting for state '%s' (current '%s')", Utils::ToString(WantedState), Utils::ToString(CurrentState));

    const HANDLE handles[] = {m_hChangedStateEvent.get(), m_hTerminationEvent.get()};

    while ( true )
    {
        //
        // if the wanted state current or already, leave
        //
        if ( CurrentState >= WantedState )
        {
            break;
        }

        //
        // Otherwise wait to be signaled
        //
        DWORD dwIndex = ::WaitForMultipleObjects(countof(handles), handles, false, INFINITE) - WAIT_OBJECT_0;
        if ( handles[dwIndex] == m_hTerminationEvent.get() )
        {
            break;
        }

        if ( handles[dwIndex] == m_hChangedStateEvent.get() )
        {
            m_bIsShuttingDown = true;
            break;
        }

        CFB::Log::perror("ManagerBase::WaitForState::WaitForSingleObject");
        return false;
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

bool
ManagerBase::NotifyTermination()
{
    auto bRes = ::SetEvent(m_hTerminationEvent.get());
    if ( false == bRes )
    {
        CFB::Log::perror("ManagerBase::NotifyTermination::SetEvent");
    }
    return bRes;
}


} // namespace CFB::Broker
