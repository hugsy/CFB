#pragma once

#include "ConnectorManager.hpp"
#include "ServiceManager.hpp"

struct GlobalContext
{
    CFB::Broker::ServiceManager ServiceManager;
    CFB::Broker::ConnectorManager ConnectorManager;
};


extern GlobalContext Globals;
