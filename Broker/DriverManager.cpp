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


Result<json>
DriverManager::ExecuteCommand(json const& Request)
{
    std::lock_guard lock(m_ManagerLock);
    json Response;
    bool bSuccess = false;
    DWORD nb      = 0;

    if ( Globals.State() <= CFB::Broker::State::ServiceManagerReady )
    {
        return Err(ErrorCode::DeviceNotInitialized);
    }


    switch ( Request.at("id").get<CFB::Comms::RequestId>() )
    {
    case CFB::Comms::RequestId::HookDriver:
    {
        auto msg         = Request.get<CFB::Comms::DriverRequest>();
        auto data_in     = (LPVOID)msg.DriverName.c_str();
        auto data_in_len = msg.DriverName.size() * sizeof(wchar_t);
        Response["success"] =
            (TRUE ==
             ::DeviceIoControl(m_hDevice.get(), IOCTL_HookDriver, data_in, data_in_len, nullptr, 0, &nb, nullptr));
        break;
    }

    case CFB::Comms::RequestId::UnhookDriver:
    {
        auto msg         = Request.get<CFB::Comms::DriverRequest>();
        auto data_in     = (LPVOID)msg.DriverName.c_str();
        auto data_in_len = msg.DriverName.size() * sizeof(wchar_t);
        Response["success"] =
            (TRUE ==
             ::DeviceIoControl(m_hDevice.get(), IOCTL_UnhookDriver, data_in, data_in_len, nullptr, 0, &nb, nullptr));
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
            for ( auto const& entry : Value(res) )
            {
                std::wstring res = L"\\driver\\" + entry.first;
                Response["body"].push_back(g_converter.to_bytes(res));
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
