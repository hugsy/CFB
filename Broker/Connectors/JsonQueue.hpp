#pragma once

#include <queue>
#include <mutex>

#include "Connectors/Base.hpp"

namespace CFB::Broker::Connectors
{

class JsonQueue : public ConnectorBase
{
public:
    JsonQueue();

    ~JsonQueue();

    std::string const
    Name() const override;

    Result<u32>
    IrpCallback(CFB::Comms::CapturedIrp const& Irp) override;

    std::unique_ptr<CFB::Comms::CapturedIrp>
    Pop();

private:
    std::queue<std::unique_ptr<CFB::Comms::CapturedIrp>> m_Queue;
    std::mutex m_Lock;
};

} // namespace CFB::Broker::Connectors
