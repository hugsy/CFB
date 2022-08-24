#include "DriverManager.hpp"

#include <codecvt>
#include <locale>

#include "BrokerUtils.hpp"
#include "Context.hpp"
#include "Error.hpp"
#include "Log.hpp"
#include "Messages.hpp"
#include "States.hpp"


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

    switch ( Request["id"].get<CFB::Comms::RequestId>() )
    {
    case CFB::Comms::RequestId::EnumerateDriverObject:
    {
        auto res = CFB::Broker::Utils::EnumerateObjectDirectory(L"\\Driver");
        if ( Success(res) )
        {
            Response["errcode"] = 0;
            std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
            for ( auto const& entry : Value(res) )
            {
                std::wstring res = L"\\driver\\" + entry.first;
                Response["body"].push_back(cvt.to_bytes(res));
            }
        }
        else
        {
            Response["errcode"] = 1;
        }
        break;
    }

    case CFB::Comms::RequestId::EnumerateDeviceObject:
    {
        auto res = CFB::Broker::Utils::EnumerateObjectDirectory(L"\\Device");
        if ( Success(res) )
        {
            Response["errcode"] = 0;
            std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
            for ( auto const& entry : Value(res) )
            {
                std::wstring res = L"\\Device\\" + entry.first;
                Response["body"].push_back(cvt.to_bytes(res));
            }
        }
        else
        {
            Response["errcode"] = 1;
        }
        break;
    }


    default:
        return Err(ErrorCode::InvalidRequestId);
    }

    return Ok(Response);
}


} // namespace CFB::Broker
