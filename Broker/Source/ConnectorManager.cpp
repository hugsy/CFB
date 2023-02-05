// clang-format off
#include "ConnectorManager.hpp"

#include "Context.hpp"
#include "Log.hpp"

#include "Connectors/Dummy.hpp" // for test
#include "Connectors/JsonQueue.hpp"
// clang-format on


namespace CFB::Broker
{

///
///@brief
///
static std::vector<std::shared_ptr<Connectors::ConnectorBase>> g_Connectors {};

///
///@brief
///
///@param Irp
///@return true
///@return false
///
bool
CallbackDispatcher(CFB::Comms::CapturedIrp const& Irp)
{
    const usize nbTotal = g_Connectors.size();
    dbg("[ConnectorManager::CallbackDispatcher] Dispatching IRP @ %llu to %u connector%s",
        Irp.Header.TimeStamp,
        nbTotal,
        PLURAL_IF(nbTotal > 1),
        &g_Connectors);


    usize nbSuccess = 0;

    for ( auto& Connector : g_Connectors )
    {
        if ( Connector->IsEnabled() == false )
        {
            continue;
        }

        dbg("[ConnectorManager::CallbackDispatcher] Sending IRP to Connector:'%s'", Connector->Name().c_str());
        if ( Success(Connector->IrpCallback(Irp)) )
        {
            nbSuccess++;
        }
    }

    dbg("[ConnectorManager::CallbackDispatcher] %u/%u executed successfully", nbSuccess, nbTotal);
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
    xdbg("Register the connector dispatcher to the IRP manager");
    if ( Globals.IrpManager()->SetCallback(&CallbackDispatcher) )
    {
        xinfo("Connector dispatcher successfully registered");

        //
        // Add new connector to that list
        //
        {
            auto conn = std::make_shared<Connectors::Dummy>();
            conn->Enable();
            g_Connectors.push_back(conn);
        }

        {
            auto conn = std::make_shared<Connectors::JsonQueue>();
            conn->Enable();
            g_Connectors.push_back(conn);
        }

        xdbg("%u connector%s registered to %p", g_Connectors.size(), PLURAL_IF(g_Connectors.size() > 1), &g_Connectors);
    }
    else
    {
        xinfo("Failed to register the connector dispatcher");
        // TODO report the error
    }

    //
    // Notify other threads that the Collector Manager is ready
    // This will also effectively start the collection by the IrpManager from the driver
    //
    SetState(CFB::Broker::State::ConnectorManagerReady);

    return Ok(true);
}


Result<std::shared_ptr<Connectors::ConnectorBase>>
ConnectorManager::GetConnectorByName(std::string_view const& ConnectorName)
{
    auto res = std::find_if(
        g_Connectors.cbegin(),
        g_Connectors.cend(),
        [&ConnectorName](auto const& Conn)
        {
            return Conn->Name() == ConnectorName;
        });

    if ( res == std::end(g_Connectors) )
    {
        return Err(ErrorCode::NotFound);
    }

    return Ok(*res);
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
