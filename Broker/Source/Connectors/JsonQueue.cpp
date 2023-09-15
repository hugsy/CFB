#include "Connectors/JsonQueue.hpp"

#include "Log.hpp"


namespace CFB::Broker::Connectors
{
JsonQueue::JsonQueue()
{
    dbg("Initializing connector '%s'", Name().c_str());
}

JsonQueue::~JsonQueue()
{
    dbg("Terminating connector '%s'", Name().c_str());
}

std::string const
JsonQueue::Name() const
{
    return "JsonQueue";
}

Result<u32>
JsonQueue::IrpCallback(CFB::Comms::CapturedIrp const& Irp)
{
    m_Queue.push(std::make_unique<CFB::Comms::CapturedIrp>(Irp));
    return Ok(0);
}

std::unique_ptr<CFB::Comms::CapturedIrp>
JsonQueue::Pop()
{
    if ( m_Queue.empty() )
    {
        return nullptr;
    }

    std::scoped_lock(m_Lock);
    auto Irp = std::move(m_Queue.front());
    m_Queue.pop();

    return Irp;
}

} // namespace CFB::Broker::Connectors
