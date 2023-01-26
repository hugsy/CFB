// clang-format off
#include "ConnectorManager.hpp"

#include "Context.hpp"
#include "Log.hpp"

#include "Connectors/Dummy.hpp"
// clang-format on


namespace Connectors = CFB::Broker::Connectors;

std::vector<std::unique_ptr<Connectors::ConnectorBase>> g_Connectors;

namespace CFB::Broker
{

ConnectorManager::ConnectorManager()
{
    //
    // Add new connector to that list
    //
    g_Connectors.push_back(std::make_unique<Connectors::Dummy>());
}


ConnectorManager::~ConnectorManager()
{
}


static bool
CallbackDispatcher(CFB::Comms::CapturedIrp const& Irp)
{
    for ( auto& Connector : g_Connectors )
    {
        dbg("[ConnectorManager::CallbackDispatcher] Sending IRP to Connector:'%s'", Connector->Name().c_str());
        Connector->IrpCallback(Irp);
    }

    return true;
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
    // Register the callback dispatcher
    //
    xdbg("Register the IRP callback dispatcher");
    Globals.IrpManager()->SetCallback(&CallbackDispatcher);

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
    WaitForState(CFB::Broker::State::Running);

    //
    // Wait for termination event
    //
    ::WaitForSingleObject(m_hTerminationEvent.get(), INFINITE);
    xdbg("TerminationEvent received");

    //
    // Propagate the notification to the other managers
    //
    SetState(CFB::Broker::State::ConnectorManagerDone);
}


} // namespace CFB::Broker
