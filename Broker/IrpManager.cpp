#include "IrpManager.hpp"

#include <chrono>
#include <thread>

#include "Context.hpp"
#include "IoctlCodes.hpp"
#include "Log.hpp"
#include "Messages.hpp"


using namespace std::literals::chrono_literals;


namespace CFB::Broker
{
IrpManager::IrpManager()
{
    //
    // Create the event shared with the driver to get new IRPs notifications
    //
    {
        wil::unique_handle hEvent(::CreateEventW(nullptr, true, false, nullptr));
        if ( !hEvent )
        {
            CFB::Log::perror("IrpManager::CreateEventW()");
            throw std::runtime_error("IrpManager()");
        }

        xdbg("Got handle to event %x", hEvent.get());
        m_hNewIrpEvent = std::move(hEvent);
    }
}


IrpManager::~IrpManager()
{
}


std::string const
IrpManager::Name()
{
    return "IrpManager";
}


Result<bool>
IrpManager::Setup()
{
    //
    // Wait for the service to be ready
    //
    WaitForState(CFB::Broker::State::ServiceManagerReady);

    //
    // Get a handle to the driver
    //
    {
        wil::unique_handle hDevice(::CreateFileW(
            CFB_USER_DEVICE_PATH,
            GENERIC_READ,
            FILE_SHARE_READ,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr));
        if ( !hDevice )
        {
            CFB::Log::perror("IrpManager::CreateFileW()");
            return Err(ErrorCode::InitializationError);
        }
        m_hDevice = std::move(hDevice);
    }
    xdbg("Got handle %x to device %S", m_hDevice.get(), CFB_USER_DEVICE_PATH);

    //
    // Share the notification event with the driver
    //
    {
        DWORD dwNbBytesReturned;
        const HANDLE hTarget = m_hNewIrpEvent.get();

        if ( ::DeviceIoControl(
                 m_hDevice.get(),
                 static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(CFB::Comms::Ioctl::SetEventPointer),
                 (LPVOID)&hTarget,
                 sizeof(hTarget),
                 nullptr,
                 0,
                 &dwNbBytesReturned,
                 nullptr) == false )
        {
            CFB::Log::perror("DeviceIoControl()");
            return Err(ErrorCode::InitializationError);
        }
    }
    xdbg("Event shared with driver");

    //
    // Notify other threads that the IRP Manager is ready
    //
    SetState(CFB::Broker::State::IrpManagerReady);

    return Ok(true);
}


void
IrpManager::Run()
{

    //
    // Wait for the collector manager to be ready so we can
    // start collecting IRPs
    //
    WaitForState(CFB::Broker::State::ConnectorManagerReady);

    xdbg("Waiting for intercepted IRP...");

    //
    // The IRP Manager waits for either a termination event or a new IRP event
    //
    const HANDLE Handles[2] = {Globals.TerminationEvent(), m_hNewIrpEvent.get()};
    bool bDoLoop            = true;
    while ( bDoLoop )
    {
        DWORD dwWaitResult = ::WaitForMultipleObjects(_countof(Handles), Handles, false, INFINITE);
        switch ( dwWaitResult )
        {
        case WAIT_OBJECT_0 + 0:
        {
            xdbg("Received termination event");
            bDoLoop = false;
            break;
        }
        case WAIT_OBJECT_0 + 1:
        {
            xdbg("New IRP data Event");
            for ( auto const& Irp : GetNextIrps() )
            {
                for ( auto const& ConnectorCb : m_Callbacks )
                {
                    ConnectorCb(Irp);
                }
            }
            break;
        }
        default:
        {
            err("WaitForMultipleObjects() returned %x", dwWaitResult);
            bDoLoop = false;
            break;
        }
        }
    }

    WaitForState(CFB::Broker::State::ConnectorManagerDone);

    //
    // Finish the IRP manager thread
    //
    SetState(CFB::Broker::State::IrpManagerDone);
}


std::vector<int>
IrpManager::GetNextIrps()
{
    std::vector<int> Irps {};

    // TODO: read ReadFile() the new IRPs from the device, store them in the vector

    //
    // Reset the event
    //
    ::ResetEvent(m_hNewIrpEvent.get());

    return Irps;
}


} // namespace CFB::Broker
