
#include "DriverManager.hpp"

#include "Context.hpp"
#include "Error.hpp"
#include "Log.hpp"
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
            throw std::runtime_error("IrpManager()");
        }

        xdbg("Got handle %x to device %S", hDevice.get(), CFB_USER_DEVICE_PATH);
        m_hDevice = std::move(hDevice);
    }

    // SetState(CFB::Broker::State::DriverManagerReady);

    return Ok(true);
}


Result<u32>
DriverManager::SendIoctl()
{
    return Ok(0);
}

/*
auto res = CFB::Broker::Utils::EnumerateObjectDirectory(L"\\Driver");
if ( Success(res) )
{
    for ( auto const& entry : Value(res) )
    {
        auto const& wname = entry.first;
        std::wcout << L"\\driver\\" << wname << std::endl;
    }
}
*/

} // namespace CFB::Broker
