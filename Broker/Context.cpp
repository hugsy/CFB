#include "Context.hpp"

#include "Log.hpp"

GlobalContext::GlobalContext() : m_State(CFB::Broker::State::Uninitialized), m_Pid(::GetCurrentProcessId())
{
    //
    // Get the broker executable absolute path
    //
    {
        std::wstring wsPath;
        wsPath.resize(MAX_PATH);
        ::GetModuleFileNameW(nullptr, wsPath.data(), MAX_PATH);
        m_BrokerPath = wsPath;
    }

    //
    // Create a termination event
    //
    {
        wil::unique_handle hEvent(::CreateEventW(nullptr, true, false, nullptr));
        if ( !hEvent )
        {
            CFB::Log::perror("CreateEventW(TerminationEvent)");
            throw std::runtime_error("GlobalContext()");
        }

        m_hTerminationEvent = std::move(hEvent);
    }

    //
    // Initialiazes the managers
    //
    m_ServiceManagerThread = std::jthread(
        [this]()
        {
            m_ServiceManager = std::make_shared<CFB::Broker::ServiceManager>();
            m_ServiceManager->Run();
        });

    m_IrpManagerThread = std::jthread(
        [this]()
        {
            m_IrpManager = std::make_shared<CFB::Broker::IrpManager>();
            m_IrpManager->Run();
        });

    m_ConnectorManagerThread = std::jthread(
        [this]()
        {
            m_ConnectorManager = std::make_shared<CFB::Broker::ConnectorManager>();
            m_ConnectorManager->Run();
        });

    m_DriverManagerThread = std::jthread(
        [this]()
        {
            m_DriverManager = std::make_shared<CFB::Broker::DriverManager>();
            m_DriverManager->Run();
        });
}


CFB::Broker::State const
GlobalContext::State() const
{
    return m_State;
}


bool
GlobalContext::SetState(CFB::Broker::State NewState)
{
    std::scoped_lock lock(m_StateMutex);
    dbg("%s -> %s", CFB::Broker::Utils::ToString(m_State), CFB::Broker::Utils::ToString(NewState));
    if ( NewState < m_State )
    {
        warn(
            "Suspicious state transition asked: Current: %s -> New: %s",
            CFB::Broker::Utils::ToString(NewState),
            CFB::Broker::Utils::ToString(m_State));
    }

    m_State = NewState;
    return true;
}

u32 const
GlobalContext::Pid() const
{
    return m_Pid;
}


fs::path const&
GlobalContext::Path() const
{
    return m_BrokerPath;
}


std::shared_ptr<CFB::Broker::ServiceManager>
GlobalContext::ServiceManager() const
{
    return m_ServiceManager;
}


const HANDLE
GlobalContext::TerminationEvent() const
{
    return m_hTerminationEvent.get();
}


bool
GlobalContext::Stop()
{
    return ::SetEvent(m_hTerminationEvent.get());
}
