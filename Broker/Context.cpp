#include "Context.hpp"

#include "Log.hpp"

GlobalContext::GlobalContext() :
    m_State(CFB::Broker::State::Uninitialized),
    m_Pid(::GetCurrentProcessId()),
    m_bIsShuttingDown(false)
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
    m_ServiceManager   = std::make_shared<CFB::Broker::ServiceManager>();
    m_IrpManager       = std::make_shared<CFB::Broker::IrpManager>();
    m_ConnectorManager = std::make_shared<CFB::Broker::ConnectorManager>();
    m_DriverManager    = std::make_shared<CFB::Broker::DriverManager>();

    //
    // Start the individual threads
    //
    m_ServiceManagerThread = std::jthread(
        [this]()
        {
            if ( Success(m_ServiceManager->Setup()) )
            {
                m_ServiceManager->Run();
            }
        });

    m_IrpManagerThread = std::jthread(
        [this]()
        {
            if ( Success(m_IrpManager->Setup()) )
            {
                m_IrpManager->Run();
            }
        });

    m_ConnectorManagerThread = std::jthread(
        [this]()
        {
            if ( Success(m_ConnectorManager->Setup()) )
            {
                m_ConnectorManager->Run();
            }
        });

    m_DriverManagerThread = std::jthread(
        [this]()
        {
            if ( Success(m_DriverManager->Setup()) )
            {
                m_DriverManager->Run();
            }
        });
}


GlobalContext::~GlobalContext()
{
    info("Destroying global context.");

    m_ServiceManagerThread.join();
    m_IrpManagerThread.join();
    m_ConnectorManagerThread.join();
    m_DriverManagerThread.join();
}


CFB::Broker::State const
GlobalContext::State() const
{
    return m_State;
}


bool
GlobalContext::SetState(CFB::Broker::State NewState)
{
    bool res = true;

    {
        std::scoped_lock lock(m_StateMutex);

        //
        // This shouldn't happen, so print a warning if it does for investigate
        //
        if ( NewState < m_State )
        {
            warn(
                "Suspicious state transition asked: Current: %s -> New: %s",
                CFB::Broker::Utils::ToString(NewState),
                CFB::Broker::Utils::ToString(m_State));
            res = false;
        }

        //
        // Apply the global state change
        //
        m_State = NewState;
    }

    //
    // Notify all managers
    //
    res &= m_ServiceManager->NotifyStateChange();
    res &= m_ConnectorManager->NotifyStateChange();
    res &= m_IrpManager->NotifyStateChange();
    res &= m_DriverManager->NotifyStateChange();

    return res;
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
    bool res = true;

    if ( !m_bIsShuttingDown )
    {
        m_bIsShuttingDown = true;

        res &= m_ServiceManager->NotifyTermination();
        res &= m_ConnectorManager->NotifyTermination();
        res &= m_IrpManager->NotifyTermination();
        res &= m_DriverManager->NotifyTermination();
        res &= (::SetEvent(m_hTerminationEvent.get()) == TRUE);
    }

    return res;
}
