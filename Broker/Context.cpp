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
    // Initialiazes the managers
    //
    m_ServiceManagerThread = std::jthread(
        [this]()
        {
            m_ServiceManager = std::make_shared<CFB::Broker::ServiceManager>();
            m_ServiceManager->Run();
        });
}

std::atomic_flag const&
GlobalContext::StateChangeFlag() const
{
    return m_AtomicStateChangeFlag;
}

CFB::Broker::State const
GlobalContext::State() const
{
    return m_State;
}

bool
GlobalContext::NotifyNewState(CFB::Broker::State NewState)
{
    auto lock = std::scoped_lock(m_Mutex);
    dbg("[Global] %s -> %s", CFB::Broker::Utils::ToString(m_State), CFB::Broker::Utils::ToString(NewState));
    m_State = NewState;
    m_AtomicStateChangeFlag.test_and_set();
    m_AtomicStateChangeFlag.notify_all();
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
