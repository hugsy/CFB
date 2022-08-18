#include "ConnectorManager.hpp"

#include "Context.hpp"

namespace CFB::Broker
{

ConnectorManager::ConnectorManager()
{
    //
    // Collect all connectors
    //
}


ConnectorManager::~ConnectorManager()
{
}


std::string const
ConnectorManager::Name()
{
    return "ConnectorManager";
}


Result<bool>
ConnectorManager::Setup()
{
    //
    // Wait for the service to be ready
    //
    WaitForState(CFB::Broker::State::IrpManagerReady);

    //
    // Register the callback for all the connectors found
    //

    //
    // Notify other threads that the Collector Manager is ready
    // This will also effectively start the collection by the IrpManager from the driver
    //
    SetState(CFB::Broker::State::ConnectorManagerReady);

    return Ok(true);
}

void
ConnectorManager::Run()
{
    //
    // Wait for termination event
    //
    ::WaitForSingleObject(m_hTerminationEvent.get(), INFINITE);

    //
    // Propagate the notification to the other managers
    //
    SetState(CFB::Broker::State::ConnectorManagerDone);
}


} // namespace CFB::Broker
