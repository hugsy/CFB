#pragma once

#include "Common.hpp"
#include "Comms.hpp"
#include "Error.hpp"

namespace CFB::Broker::Connectors
{
class ConnectorBase
{
public:
    ConnectorBase() : m_Enabled {false}
    {
    }

    ~ConnectorBase()
    {
    }

    virtual std::string const
    Name() const = 0;

    void
    Enable()
    {
        m_Enabled = true;
    }

    void
    Disable()
    {
        m_Enabled = false;
    }

    bool
    IsEnabled() const
    {
        return m_Enabled;
    }

    virtual Result<u32>
    IrpCallback(CFB::Comms::CapturedIrp const& Irp) = 0;

protected:
    bool m_Enabled;
};
} // namespace CFB::Broker::Connectors
