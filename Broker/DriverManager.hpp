#pragma once


#include <wil/resource.h>

#include "Common.hpp"
#include "ManagerBase.hpp"

namespace CFB::Broker
{
class DriverManager : ManagerBase
{
public:
    DriverManager()
    {
    }

    ~DriverManager()
    {
    }

    void
    Run()
    {
    }

private:
    wil::unique_handle hDevice;
};

} // namespace CFB::Broker
