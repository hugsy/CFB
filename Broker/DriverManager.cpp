#include "DriverManager.hpp"

#include "Log.hpp"

namespace CFB::Broker
{
DriverManager::DriverManager()
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
}

DriverManager::~DriverManager()
{
}

void
DriverManager::Run()
{
    WaitForState(CFB::Broker::State::IrpManagerDone);
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
