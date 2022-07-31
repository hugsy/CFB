#pragma once

namespace CFB::Broker
{
enum class State : int
{
    Uninitialized = 1,
    ServiceManagerReady,
    DriverManagerReady,
    ConnectorManagerReady,
    Running,
    ConnectorManagerDone,
    DriverManagerDone,
    ServiceManagerDone,
    Finished
};
} // namespace CFB::Broker
