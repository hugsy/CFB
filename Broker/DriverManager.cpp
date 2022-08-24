#include "DriverManager.hpp"

#include <codecvt>
#include <locale>

#include "BrokerUtils.hpp"
#include "Context.hpp"
#include "Error.hpp"
#include "IoctlCodes.hpp"
#include "Log.hpp"
#include "States.hpp"


static std::wstring_convert<std::codecvt_utf8<wchar_t>> g_converter;

namespace CFB::Broker
{
DriverManager::DriverManager()
{
}

DriverManager::~DriverManager()
{
}


std::string const
DriverManager::Name()
{
    return "DriverManager";
}


void
DriverManager::Run()
{
    m_Listener.RunForever();
}


Result<bool>
DriverManager::Setup()
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
            CFB::Log::perror("DriverManager::CreateFileW()");
            return Err(ErrorCode::InitializationError);
        }

        xdbg("Got handle %x to device %S", hDevice.get(), CFB_USER_DEVICE_PATH);
        m_hDevice = std::move(hDevice);
    }

    // SetState(CFB::Broker::State::DriverManagerReady);

    return Ok(true);
}


Result<CFB::Comms::DriverResponse>
DriverManager::SendIoctlRequest(CFB::Comms::DriverRequest const& msg)
{
    DWORD nbBytesReturned;
    CFB::Comms::DriverResponse Response;

    bool bSuccess = ::DeviceIoControl(
        m_hDevice.get(),
        IOCTL_ControlDriver,
        (LPVOID)&msg,
        sizeof(CFB::Comms::DriverRequest),
        nullptr,
        0,
        &nbBytesReturned,
        nullptr);
    info("DriverManager::SendIoctlRequest() returned %s", boolstr(bSuccess));

    return Ok(Response);
}


Result<json>
DriverManager::ExecuteCommand(json const& Request)
{
    std::lock_guard lock(m_ManagerLock);
    json Response;

    if ( Globals.State() <= CFB::Broker::State::ServiceManagerReady )
    {
        return Err(ErrorCode::DeviceNotInitialized);
    }

    switch ( Request["id"].get<CFB::Comms::RequestId>() )
    {
    case CFB::Comms::RequestId::HookDriver:
    {
        CFB::Comms::DriverRequest msg;
        msg.Id                    = CFB::Comms::RequestId::HookDriver;
        std::string driver_name   = Request["driver_name"].get<std::string>();
        std::wstring wdriver_name = g_converter.from_bytes(driver_name);
        ::RtlCopyMemory(msg.Data.DriverName, wdriver_name.data(), wdriver_name.size() * 2);
        Response["success"] = Success(SendIoctlRequest(msg));
        break;
    }


    case CFB::Comms::RequestId::UnhookDriver:
    {
        CFB::Comms::DriverRequest msg;
        msg.Id                    = CFB::Comms::RequestId::UnhookDriver;
        std::string driver_name   = Request["driver_name"].get<std::string>();
        std::wstring wdriver_name = g_converter.from_bytes(driver_name);
        ::RtlCopyMemory(msg.Data.DriverName, wdriver_name.data(), wdriver_name.size() * 2);
        Response["success"] = Success(SendIoctlRequest(msg));
        break;
    }


    case CFB::Comms::RequestId::GetNumberOfDrivers:
    {
        break;
    }


    case CFB::Comms::RequestId::GetNamesOfDrivers:
    {
        break;
    }


    case CFB::Comms::RequestId::GetDriverInfo:
    {
        break;
    }


    case CFB::Comms::RequestId::EnableMonitoring:
    {
        break;
    }


    case CFB::Comms::RequestId::DisableMonitoring:
    {
        break;
    }


    case CFB::Comms::RequestId::StoreTestCase:
    {
        break;
    }


    case CFB::Comms::RequestId::EnableDriver:
    {
        break;
    }


    case CFB::Comms::RequestId::DisableDriver:
    {
        break;
    }

    case CFB::Comms::RequestId::EnumerateDriverObject:
    {
        auto res = CFB::Broker::Utils::EnumerateObjectDirectory(L"\\Driver");
        if ( Success(res) )
        {
            Response["success"] = true;
            std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
            for ( auto const& entry : Value(res) )
            {
                std::wstring res = L"\\driver\\" + entry.first;
                Response["body"].push_back(cvt.to_bytes(res));
            }
        }
        else
        {
            Response["success"] = false;
        }
        break;
    }

    case CFB::Comms::RequestId::EnumerateDeviceObject:
    {
        auto res = CFB::Broker::Utils::EnumerateObjectDirectory(L"\\Device");
        if ( Success(res) )
        {
            Response["success"] = true;

            for ( auto const& entry : Value(res) )
            {
                std::wstring res = L"\\Device\\" + entry.first;
                Response["body"].push_back(g_converter.to_bytes(res));
            }
        }
        else
        {
            Response["success"] = false;
        }
        break;
    }

    default:
        return Err(ErrorCode::InvalidRequestId);
    }

    return Ok(Response);
}


} // namespace CFB::Broker
