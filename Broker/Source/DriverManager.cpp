#define CFB_NS "[CFB::Broker::DriverManager]"

// clang-format off
#include "DriverManager.hpp"

#include <codecvt>
#include <locale>

#include "BrokerUtils.hpp"
#include "Context.hpp"
#include "Error.hpp"
#include "IoctlCodes.hpp"
#include "Log.hpp"
#include "Messages.hpp"
#include "States.hpp"
#include "Utils.hpp"
#include "Comms.hpp"

#include "Connectors/JsonQueue.hpp"
// clang-format on

namespace CFB::Broker
{
DriverManager::DriverManager() : m_RequestNumber {0}
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
    WaitForState(CFB::Broker::State::Running);

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

        dbg("Got handle %x to device %S", hDevice.get(), CFB_USER_DEVICE_PATH);
        m_hDevice = std::move(hDevice);
    }

    return Ok(true);
}


Result<json>
DriverManager::ExecuteCommand(json const& Request)
{
    std::lock_guard lock(m_ManagerLock);
    json Response;
    bool bSuccess    = false;
    DWORD nbReturned = 0;

    if ( Globals.State() <= CFB::Broker::State::ServiceManagerReady )
    {
        return Err(ErrorCode::DeviceNotInitialized);
    }

    auto RequestId      = Request.at("id").get<CFB::Comms::RequestId>();
    Response["success"] = false;

    InterlockedIncrement64((long long*)&m_RequestNumber);

    if ( RequestId != CFB::Comms::RequestId::GetPendingIrp )
        dbg("New request %s => ID=%llu", CFB::Utils::ToString(RequestId).c_str(), m_RequestNumber);

    switch ( RequestId )
    {
    case CFB::Comms::RequestId::HookDriver:
    {
        auto msg         = Request.get<CFB::Comms::DriverRequest>();
        auto data_in     = (LPVOID)msg.DriverName.c_str();
        auto data_in_len = msg.DriverName.size() * sizeof(wchar_t);
        Response["success"] =
            (TRUE == ::DeviceIoControl(
                         m_hDevice.get(),
                         static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(Comms::Ioctl::HookDriver),
                         data_in,
                         data_in_len,
                         nullptr,
                         0,
                         &nbReturned,
                         nullptr));
        break;
    }

    case CFB::Comms::RequestId::UnhookDriver:
    {
        auto msg         = Request.get<CFB::Comms::DriverRequest>();
        auto data_in     = (LPVOID)msg.DriverName.c_str();
        auto data_in_len = msg.DriverName.size() * sizeof(wchar_t);
        Response["success"] =
            (TRUE == ::DeviceIoControl(
                         m_hDevice.get(),
                         static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(Comms::Ioctl::UnhookDriver),
                         data_in,
                         data_in_len,
                         nullptr,
                         0,
                         &nbReturned,
                         nullptr));
        break;
    }

    case CFB::Comms::RequestId::GetNumberOfDrivers:
    {
        Response["success"] =
            (TRUE == ::DeviceIoControl(
                         m_hDevice.get(),
                         static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(Comms::Ioctl::GetNumberOfDrivers),
                         nullptr,
                         0,
                         nullptr,
                         0,
                         &nbReturned,
                         nullptr));
        Response["value"] = nbReturned;
        break;
    }

    case CFB::Comms::RequestId::EnableMonitoring:
    {
        auto msg         = Request.get<CFB::Comms::DriverRequest>();
        auto data_in     = (LPVOID)msg.DriverName.c_str();
        auto data_in_len = msg.DriverName.size() * sizeof(wchar_t);
        Response["success"] =
            (TRUE == ::DeviceIoControl(
                         m_hDevice.get(),
                         static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(Comms::Ioctl::EnableMonitoring),
                         data_in,
                         data_in_len,
                         nullptr,
                         0,
                         &nbReturned,
                         nullptr));
        break;
    }

    case CFB::Comms::RequestId::DisableMonitoring:
    {
        auto msg         = Request.get<CFB::Comms::DriverRequest>();
        auto data_in     = (LPVOID)msg.DriverName.c_str();
        auto data_in_len = msg.DriverName.size() * sizeof(wchar_t);
        Response["success"] =
            (TRUE == ::DeviceIoControl(
                         m_hDevice.get(),
                         static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(Comms::Ioctl::DisableMonitoring),
                         data_in,
                         data_in_len,
                         nullptr,
                         0,
                         &nbReturned,
                         nullptr));
        break;
    }

    case CFB::Comms::RequestId::GetDriverInfo:
    {
        Response["success"] = false;
        Response["reason"]  = "NotImplemented";
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
                Response["body"].push_back(CFB::Utils::ToString(res));
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
                Response["body"].push_back(CFB::Utils::ToString(res));
            }
        }
        else
        {
            Response["success"] = false;
        }
        break;
    }

    case CFB::Comms::RequestId::EnumerateMinifilterObject:
    {
        auto res = CFB::Broker::Utils::EnumerateObjectDirectory(L"\\FileSystem");
        if ( Success(res) )
        {
            Response["success"] = true;

            for ( auto const& entry : Value(res) )
            {
                std::wstring res = L"\\FileSystem\\" + entry.first;
                Response["body"].push_back(CFB::Utils::ToString(res));
            }
        }
        else
        {
            Response["success"] = false;
        }
        break;
    }

    case CFB::Comms::RequestId::GetPendingIrp:
    {
        auto msg = Request.get<CFB::Comms::DriverRequest>();
        if ( msg.NumberOfIrp == 0 )
        {
            Response["success"] = false;
            Response["reason"]  = "Invalid Parameter";
            break;
        }

        //
        //
        //
        auto res = Globals.ConnectorManager()->GetConnectorByName("JsonQueue");
        if ( Failed(res) )
        {
            Response["success"] = false;
            Response["reason"]  = "Cannot find connector";
            break;
        }

        auto const ConnBase = Value(res);
        auto const Conn     = reinterpret_cast<Connectors::JsonQueue*>(ConnBase.get());
        usize nb            = 0;
        while ( true )
        {
            std::unique_ptr<CFB::Comms::CapturedIrp> Irp = std::move(Conn->Pop());
            if ( !Irp )
            {
                break;
            }

            nb++;
            Response["body"].push_back(json(*Irp));
        }

        Response["success"]       = true;
        Response["number_of_irp"] = nb;
        break;
    }

    default:
        Response["success"] = false;
        Response["reason"]  = "Invalid Request ID";
        break;
    }

    // if ( RequestId != CFB::Comms::RequestId::GetPendingIrp )
    {
        info(
            "Request[%llu] %s => %s",
            m_RequestNumber,
            CFB::Utils::ToString(RequestId).c_str(),
            boolstr(Response["success"]));

        info("Request[%llu] => %s", m_RequestNumber, Response.dump().c_str());
    }

    return Ok(Response);
}


} // namespace CFB::Broker
