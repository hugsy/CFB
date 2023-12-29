#define CFB_NS "[CFB::Broker::IrpManager]"

#include "IrpManager.hpp"

#include <chrono>
#include <thread>

#include "Context.hpp"
#include "IoctlCodes.hpp"
#include "Log.hpp"
#include "Messages.hpp"
#include "Utils.hpp"


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

        dbg("Got handle to event %x", hEvent.get());
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
    dbg("Got handle %x to device %S", m_hDevice.get(), CFB_USER_DEVICE_PATH);

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
    dbg("Event shared with driver");

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
    WaitForState(CFB::Broker::State::Running);

    info("Waiting for intercepted IRP...");

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
            dbg("Received termination event");
            bDoLoop = false;
            break;
        }
        case WAIT_OBJECT_0 + 1:
        {
            dbg("Received new IRP data event");

            for ( auto const& Irp : GetNextIrps() )
            {
                //
                // For each new IRP, pass it on to the ConnectorManager
                //
                if ( !m_CallbackDispatcher )
                {
                    warn("No connector callback dispatcher registered");
                    break;
                }

                m_CallbackDispatcher(Irp);
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

    //
    // Finish the IRP manager thread
    //
    SetState(CFB::Broker::State::IrpManagerDone);
}


std::vector<CFB::Comms::CapturedIrp>
IrpManager::GetNextIrps()
{
    std::vector<CFB::Comms::CapturedIrp> Irps {};
    std::unique_ptr<u8[]> RawData;
    DWORD dataLength = 1;

    //
    // Poll data matching length requirement
    //
    while ( true )
    {
        DWORD nbDataRead = 0;
        RawData          = std::make_unique<u8[]>(dataLength);
        bool bRes        = ::ReadFile(m_hDevice.get(), RawData.get(), dataLength, &nbDataRead, nullptr);
        if ( bRes )
        {
            dbg("%uB data fetched, %uB allocated", nbDataRead, dataLength);
            dataLength = nbDataRead;
            break;
        }

        if ( ::GetLastError() == ERROR_INSUFFICIENT_BUFFER )
        {
            dataLength *= 2;
            continue;
        }

        CFB::Log::perror("ReadFile() failed");
        ::ResetEvent(m_hNewIrpEvent.get());
        return {};
    }

    if ( dataLength < sizeof(CFB::Comms::CapturedIrpHeader) )
    {
        warn("Event was set but not enough data was fetched");
        ::ResetEvent(m_hNewIrpEvent.get());
        return {};
    }

    //
    // Parse it
    //
    {
        usize offset = 0;

        while ( offset < dataLength )
        {
            auto const Header = reinterpret_cast<CFB::Comms::CapturedIrpHeader*>(RawData.get() + offset);
            dbg("Parsing from offset %d", offset);

            if ( offset + sizeof(CFB::Comms::CapturedIrpHeader) > dataLength )
            {
                warn(
                    "Out-of-bound Header access: offset=%llu + sizeof(header)=%llu, dataLength=%llu ",
                    offset,
                    sizeof(CFB::Comms::CapturedIrpHeader),
                    dataLength);
                break;
            }

            CFB::Comms::CapturedIrp Irp {};
            Irp.Header.TimeStamp          = Header->TimeStamp;
            Irp.Header.Irql               = Header->Irql;
            Irp.Header.Type               = Header->Type;
            Irp.Header.MajorFunction      = Header->MajorFunction;
            Irp.Header.MinorFunction      = Header->MinorFunction;
            Irp.Header.IoctlCode          = Header->IoctlCode;
            Irp.Header.Pid                = Header->Pid;
            Irp.Header.Tid                = Header->Tid;
            Irp.Header.Status             = Header->Status;
            Irp.Header.InputBufferLength  = Header->InputBufferLength;
            Irp.Header.OutputBufferLength = Header->OutputBufferLength;
            ::memcpy(Irp.Header.DriverName, Header->DriverName, sizeof(Header->DriverName));
            ::memcpy(Irp.Header.DeviceName, Header->DeviceName, sizeof(Header->DeviceName));
            ::memcpy(Irp.Header.ProcessName, Header->ProcessName, sizeof(Header->ProcessName));
            offset += sizeof(CFB::Comms::CapturedIrpHeader);

            if ( offset + Irp.Header.InputBufferLength <= dataLength )
            {
                auto const InputBuffer = RawData.get() + offset;
                Irp.InputBuffer.resize(Irp.Header.InputBufferLength);
                ::memcpy(Irp.InputBuffer.data(), InputBuffer, Irp.Header.InputBufferLength);
                offset += Irp.Header.InputBufferLength;
            }

            if ( offset + Irp.Header.OutputBufferLength <= dataLength )
            {
                auto const OutputBuffer = RawData.get() + offset;
                Irp.OutputBuffer.resize(Irp.Header.OutputBufferLength);
                ::memcpy(Irp.OutputBuffer.data(), OutputBuffer, Irp.Header.OutputBufferLength);
                offset += Irp.Header.OutputBufferLength;
            }

            dbg("New IRP (DeviceName='%S', Type=%s, InputBufferLength=%u, OutputBufferLength=%u) pushed to queue",
                Irp.Header.DeviceName,
                CFB::Utils::IrpMajorToString(Irp.Header.MajorFunction),
                Irp.Header.InputBufferLength,
                Irp.Header.OutputBufferLength);

            Irps.push_back(std::move(Irp));
        }
    }

    dbg("%d IRPs pulled from driver", Irps.size());

    //
    // Reset the event
    //
    ::ResetEvent(m_hNewIrpEvent.get());

    return Irps;
}


bool
IrpManager::SetCallback(std::function<bool(CFB::Comms::CapturedIrp const&)> cb)
{
    std::scoped_lock(m_CallbackLock);
    m_CallbackDispatcher = cb;
    dbg("Callback %p added", cb);
    return true;
}
} // namespace CFB::Broker
