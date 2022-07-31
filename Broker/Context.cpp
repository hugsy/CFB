#include "Context.hpp"

#include "Log.hpp"

GlobalContext::GlobalContext() : m_State(CFB::Broker::State::Uninitialized)
{
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
    dbg("[Global] State %d -> %d", m_State, NewState);
    auto m_State = NewState;
    m_AtomicStateChangeFlag.test_and_set();
    m_AtomicStateChangeFlag.notify_all();
    return true;
}
