#define NOMINMAX
#undef _UNICODE
#undef UNICODE

// clang-format off
#include <algorithm>
#include <codecvt>
#include <iostream>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <argparse/argparse.hpp>
#include <wil/resource.h>
// clang-format on

#define CFB_NS "[CFB::Test::DriverClient]"

#include "Common.hpp"
#include "Comms.hpp"
#include "IoctlCodes.hpp"
#include "Log.hpp"
#include "Utils.hpp"


namespace Driver
{
bool
Hook(HANDLE hFile, std::string const& arg)
{
    DWORD nb                    = 0;
    std::wstring driver_name    = CFB::Utils::ToWideString(arg);
    const usize driver_name_len = std::min(driver_name.length() * 2, (usize)CFB_DRIVER_MAX_PATH);

    bool bSuccess = ::DeviceIoControl(
        hFile,
        static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(CFB::Comms::Ioctl::HookDriver),
        driver_name.data(),
        driver_name_len,
        nullptr,
        0,
        &nb,
        nullptr);
    info("HookDriver() returned %s", boolstr(bSuccess));

    if ( !bSuccess )
        return false;

    return true;
}

bool
Unhook(HANDLE hFile, std::string const& arg)
{
    DWORD nb                    = 0;
    std::wstring driver_name    = CFB::Utils::ToWideString(arg);
    const usize driver_name_len = std::min(driver_name.length() * 2, (usize)CFB_DRIVER_MAX_PATH);

    bool bSuccess = ::DeviceIoControl(
        hFile,
        static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(CFB::Comms::Ioctl::UnhookDriver),
        driver_name.data(),
        driver_name_len,
        nullptr,
        0,
        &nb,
        nullptr);
    info("UnhookDriver() returned %s", boolstr(bSuccess));

    if ( !bSuccess )
        return false;

    return true;
}


HANDLE
SetNotificationEvent(HANDLE hFile)
{
    DWORD nbBytesReturned = 0;
    HANDLE hEvent         = ::CreateEventA(nullptr, true, false, "CFB_IRP_EVENT");

    bool bSuccess = ::DeviceIoControl(
        hFile,
        static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(CFB::Comms::Ioctl::SetEventPointer),
        &hEvent,
        sizeof(HANDLE),
        nullptr,
        0,
        &nbBytesReturned,
        nullptr);
    info("SetEventPointer() returned %s", boolstr(bSuccess));

    return hEvent;
}


bool
GetNumberOfDrivers(HANDLE hFile)
{
    DWORD nbBytesReturned = 0;
    bool bSuccess         = ::DeviceIoControl(
        hFile,
        static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(CFB::Comms::Ioctl::GetNumberOfDrivers),
        nullptr,
        0,
        nullptr,
        0,
        &nbBytesReturned,
        nullptr);
    info("GetNumberOfDrivers() returned %s", boolstr(bSuccess));

    if ( bSuccess )
    {
        ok("There's currently %d driver%s hooked", nbBytesReturned, PLURAL_IF(nbBytesReturned > 1));
    }

    return bSuccess;
}

bool
ToggleMonitoring(HANDLE hFile, std::string const& arg, bool enable)
{
    std::wstring driver_name     = CFB::Utils::ToWideString(arg);
    const usize driver_name_len  = driver_name.length() * 2;
    const usize msglen           = std::min(driver_name_len, (usize)CFB_DRIVER_MAX_PATH);
    const CFB::Comms::Ioctl code = enable ? CFB::Comms::Ioctl::EnableMonitoring : CFB::Comms::Ioctl::DisableMonitoring;

    DWORD nbBytesReturned = 0;
    bool bSuccess         = ::DeviceIoControl(
        hFile,
        static_cast<std::underlying_type<CFB::Comms::Ioctl>::type>(code),
        driver_name.data(),
        msglen,
        nullptr,
        0,
        &nbBytesReturned,
        nullptr);
    info("ToggleMonitoring('%S', enable=%s) returned %s", driver_name.c_str(), boolstr(enable), boolstr(bSuccess));

    return bSuccess;
}


bool
SendData(std::string const& device_name, const u32 ioctl, std::vector<u8>& buffer_in)
{
    wil::unique_handle hFile(::CreateFileA(
        device_name.c_str(),
        GENERIC_WRITE | GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr));
    if ( !hFile )
    {
        err("CreateFileA(%s) failed", device_name.c_str());
        return false;
    }

    DWORD nbBytesReturned = 0;
    bool bSuccess         = ::DeviceIoControl(
        hFile.get(),
        ioctl,
        buffer_in.data(),
        buffer_in.size(),
        nullptr,
        0,
        &nbBytesReturned,
        nullptr);
    info("SendData('%s', %dB) returned %s", device_name.c_str(), buffer_in.size(), boolstr(bSuccess));

    return bSuccess;
}

std::optional<u32>
ReceiveData(HANDLE hFile)
{
    DWORD expectedDataLength = 1024;

    while ( true )
    {
        const DWORD dataLength = expectedDataLength;
        auto data              = std::make_unique<u8[]>(dataLength);
        bool bRes              = ::ReadFile(hFile, data.get(), dataLength, &expectedDataLength, nullptr);
        info("ReceiveData(data, %d) = %s", dataLength, boolstr(bRes));
        if ( bRes == false )
        {
            if ( ::GetLastError() == ERROR_INSUFFICIENT_BUFFER )
            {
                expectedDataLength = dataLength * 2;
                continue;
            }
            err("ReadFile() failed, GLE %#x", ::GetLastError());
            return std::nullopt;
        }

        CFB::Utils::Hexdump(data.get(), dataLength);
        return dataLength;
    }
}

} // namespace Driver

int
main(int argc, const char** argv)
{
    argparse::ArgumentParser program("DriverClient");

    const std::vector<std::string> valid_actions =
        {"hook", "unhook", "size", "monitor", "unmonitor", "send", "recv", "session"};

    program.add_argument("--action")
        .default_value(std::string("hook"))
        .action(
            [&valid_actions](const std::string& value)
            {
                if ( std::find(valid_actions.begin(), valid_actions.end(), value) != valid_actions.end() )
                {
                    return value;
                }
                throw std::runtime_error("invalid action");
            });

    program.add_argument("--driver").default_value(std::string("\\driver\\hevd"));
    program.add_argument("--device").default_value(std::string("\\\\.\\HackSysExtremeVulnerableDriver"));
    program.add_argument("--ioctl").scan<'i', int>().default_value(0x222003); // BUFFER_OVERFLOW_STACK

    try
    {
        program.parse_args(argc, argv);
    }
    catch ( const std::runtime_error& err )
    {
        std::cerr << err.what() << "\n" << program;
        std::exit(1);
    }

    const auto action      = program.get<std::string>("--action");
    const auto driver_name = program.get<std::string>("--driver");
    const auto device_name = program.get<std::string>("--device");
    const auto ioctl       = program.get<int>("--ioctl");

    wil::unique_handle hEvent;

    info("Getting a handle to '%S'", CFB_DEVICE_PATH);
    wil::unique_handle hFile(
        ::CreateFileW(CFB_USER_DEVICE_PATH, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr));
    if ( !hFile )
    {
        err("Failed to open '%S'", CFB_DEVICE_NAME);
        return -1;
    }

    ok("Got handle to '%S' as %p", CFB_DEVICE_PATH, hFile.get());

    if ( action == "hook" )
    {
        Driver::Hook(hFile.get(), driver_name);
    }
    else if ( action == "unhook" )
    {
        Driver::Unhook(hFile.get(), driver_name);
    }
    else if ( action == "size" )
    {
        Driver::GetNumberOfDrivers(hFile.get());
    }
    else if ( action == "monitor" )
    {
        Driver::ToggleMonitoring(hFile.get(), driver_name, true);
    }
    else if ( action == "unmonitor" )
    {
        Driver::ToggleMonitoring(hFile.get(), driver_name, false);
    }
    else if ( action == "send" )
    {
        const usize sz {0x100};
        auto buffer_in = std::vector<u8>(sz);
        ::memset(buffer_in.data(), 'A', sz);
        Driver::SendData(device_name, ioctl, buffer_in);
    }
    else if ( action == "recv" )
    {
        Driver::ReceiveData(hFile.get());
    }
    else if ( action == "session" )
    {
        Driver::Hook(hFile.get(), driver_name);

        Driver::GetNumberOfDrivers(hFile.get());

        hEvent = wil::unique_handle(Driver::SetNotificationEvent(hFile.get()));

        Driver::ToggleMonitoring(hFile.get(), driver_name, true);

        for ( int i = 0; i < 2; i++ )
        {
            const usize sz {0x100};
            auto buffer_in  = std::vector<u8>(sz);
            auto buffer_out = std::vector<u8>(sz);
            ::memset(buffer_in.data(), 'A', sz);

            Driver::SendData(device_name, ioctl, buffer_in);

            u32 Status = ::WaitForSingleObject(hEvent.get(), 1 * 1000);
            switch ( Status )
            {
            case WAIT_OBJECT_0:
                ok("New data event received");
                break;

            case WAIT_TIMEOUT:
                warn("WaitForSingleObject() timed out");
                continue;

            default:
                err("WaitForSingleObject() returned %#x", Status);
                i = 10000000;
                continue;
            }

            Driver::ReceiveData(hFile.get());
        }

        Driver::ToggleMonitoring(hFile.get(), driver_name, false);

        Driver::Unhook(hFile.get(), driver_name);
    }
    else
    {
        err("Unknown action %S", action.c_str());
    }

    return 0;
}
