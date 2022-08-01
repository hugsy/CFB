#include "ConnectorManager.hpp"

#include "Context.hpp"

namespace CFB::Broker
{

ConnectorManager::ConnectorManager()
{
    //
    // Collect all connectors
    //


    //
    // Wait for the service to be ready
    //
    WaitForState(CFB::Broker::State::IrpManagerReady);

    //
    // Register the callback for all the connectors found
    //
}


ConnectorManager::~ConnectorManager()
{
}


void
ConnectorManager::Run()
{
    //
    // Notify other threads that the Collector Manager is ready
    // This will also effectively start the collection by the IrpManager from the driver
    //
    NotifyNewState(CFB::Broker::State::ConnectorManagerReady);

    //
    // Wait for termination event
    //
    ::WaitForSingleObject(Globals.TerminationEvent(), INFINITE);

    //
    // Propagate the notification to the other managers
    //
    NotifyNewState(CFB::Broker::State::ConnectorManagerDone);
}


} // namespace CFB::Broker
