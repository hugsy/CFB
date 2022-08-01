#include "IrpManager.hpp"

#include <chrono>
#include <thread>

#include "IoctlCodes.hpp"
#include "Log.hpp"

using namespace std::literals::chrono_literals;


namespace CFB::Broker
{
IrpManager::IrpManager()
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
            CFB::Log::perror("CreateFileW()");
            throw std::runtime_error("IrpManager()");
        }

        dbg("Got handle to device %x", hDevice.get());
        m_hDevice = std::move(hDevice);
    }

    //
    // Create an event
    //
    {
        wil::unique_handle hNewIrpEvent(::CreateEventW(nullptr, true, false, nullptr));
        if ( !hNewIrpEvent )
        {
            CFB::Log::perror("CreateEventW()");
            throw std::runtime_error("IrpManager()");
        }

        dbg("Got handle to event %x", hNewIrpEvent.get());
        m_hNewIrpEvent = std::move(hNewIrpEvent);
    }

    //
    // Share the notification event with the driver
    //
    {
        DWORD dwNbBytesReturned;
        HANDLE hEvent = m_hNewIrpEvent.get();

        if ( ::DeviceIoControl(
                 m_hDevice.get(),
                 IOCTL_SetEventPointer,
                 &hEvent,
                 sizeof(HANDLE),
                 nullptr,
                 0,
                 &dwNbBytesReturned,
                 nullptr) == false )
        {
            CFB::Log::perror("DeviceIoControl()");
            throw std::runtime_error("IrpManager()");
        }
    }

    dbg("[TID=%x] Event shared with driver", std::this_thread::get_id());
}

IrpManager::~IrpManager()
{
}

void
IrpManager::Run()
{
    //
    // Notify other threads that the IRP Manager is ready
    //
    NotifyNewState(CFB::Broker::State::IrpManagerReady);

    //
    // Wait for the collector manager to be ready so we can
    // start collecting IRPs
    //
    WaitForState(CFB::Broker::State::ConnectorManagerReady);


    bool bDoLoop            = true;
    const HANDLE Handles[1] = {m_hNewIrpEvent.get()}; // TODO: add a termination handle
    while ( bDoLoop )
    {
        DWORD dwWaitResult = ::WaitForMultipleObjects(_countof(Handles), Handles, false, INFINITE);
        switch ( dwWaitResult )
        {
        case WAIT_OBJECT_0 + 0:
        {
            dbg("New IRP data Event");

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
            bDoLoop = false;
            break;
        }
        }
    }

    // WaitForState(CFB::Broker::State::ConnectorManagerDone);

    //
    // Finish the IRP manager thread
    //
    NotifyNewState(CFB::Broker::State::IrpManagerDone);
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
