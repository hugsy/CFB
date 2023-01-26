#pragma once

#include "Connectors/Base.hpp"

namespace CFB::Broker::Connectors
{

class Dummy : public ConnectorBase
{
public:
    Dummy()
    {
    }

    ~Dummy()
    {
    }

    std::string const
    Name() const override;

    Result<u32>
    IrpCallback(CFB::Comms::CapturedIrp const& Irp) override;

private:
};

} // namespace CFB::Broker::Connectors
