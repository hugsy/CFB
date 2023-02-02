#pragma once

#include "Common.hpp"
#include "Comms.hpp"
#include "Error.hpp"

namespace CFB::Broker::Connectors
{
class ConnectorBase
{
public:
    ConnectorBase()
    {
    }

    ~ConnectorBase()
    {
    }

    virtual std::string const
    Name() const = 0;

    virtual Result<u32>
    IrpCallback(CFB::Comms::CapturedIrp const& Irp) = 0;

protected:
private:
};
} // namespace CFB::Broker::Connectors
