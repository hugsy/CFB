#pragma once

#include <wil/resource.h>

#include "Common.hpp"
#include "ManagerBase.hpp"

namespace CFB::Broker
{

class IrpManager : ManagerBase
{
public:
    IrpManager()
    {
    }

    ~IrpManager()
    {
    }

    void
    Run()
    {
    }

private:
    wil::unique_handle hDevice;
    wil::unique_handle hNewIrpEvent;
};

} // namespace CFB::Broker
