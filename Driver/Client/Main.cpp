#define NOMINMAX
#undef _UNICODE
#undef UNICODE

// clang-format off
#include <windows.h>

#include <algorithm>
#include <codecvt>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <argparse.hpp>
#include <wil/resource.h>

#include "Common.hpp"
#include "IoctlCodes.hpp"
#include "Log.hpp"

// clang-format on

#define PLURAL_IF(x) ((x) ? "s" : "")

#pragma warning(push)
#pragma warning(disable : 4244) // bad but only for tests

static std::wstring_convert<std::codecvt_utf8<wchar_t>> g_converter;

std::string
ws2s(std::wstring const& ws)
{
    return g_converter.to_bytes(ws);
}

std::wstring
s2ws(std::string const& s)
{
    return g_converter.from_bytes(s);
}
#pragma warning(pop)

bool
hook_driver(HANDLE hFile, std::string const& arg)
{
    DWORD nb                    = 0;
    std::wstring driver_name    = s2ws(arg);
    const usize driver_name_len = std::min(driver_name.length() * 2, (usize)CFB_DRIVER_MAX_PATH);

    bool bSuccess =
        ::DeviceIoControl(hFile, IOCTL_HookDriver, driver_name.data(), driver_name_len, nullptr, 0, &nb, nullptr);
    info("HookDriver() returned %s", boolstr(bSuccess));

    if ( !bSuccess )
        return false;

    return true;
}

bool
unhook_driver(HANDLE hFile, std::string const& arg)
{
    DWORD nb                    = 0;
    std::wstring driver_name    = s2ws(arg);
    const usize driver_name_len = std::min(driver_name.length() * 2, (usize)CFB_DRIVER_MAX_PATH);

    bool bSuccess =
        ::DeviceIoControl(hFile, IOCTL_UnhookDriver, driver_name.data(), driver_name_len, nullptr, 0, &nb, nullptr);
    info("UnhookDriver() returned %s", boolstr(bSuccess));

    if ( !bSuccess )
        return false;

    return true;
}


HANDLE
set_notif_handle(HANDLE hFile)
{
    DWORD nbBytesReturned = 0;
    HANDLE hEvent         = ::CreateEventA(nullptr, true, false, "CFB_IRP_EVENT");

    bool bSuccess =
        ::DeviceIoControl(hFile, IOCTL_SetEventPointer, &hEvent, sizeof(HANDLE), nullptr, 0, &nbBytesReturned, nullptr);
    info("SetEventPointer() returned %s", boolstr(bSuccess));

    if ( bSuccess )
    {
        info("Waiting for event...");
        ::WaitForSingleObject(hFile, INFINITE);
        ok("Event received");
    }

    return hEvent;
}


bool
get_size(HANDLE hFile)
{
    DWORD nbBytesReturned = 0;
    bool bSuccess =
        ::DeviceIoControl(hFile, IOCTL_GetNumberOfDrivers, nullptr, 0, nullptr, 0, &nbBytesReturned, nullptr);
    info("GetNumberOfDrivers() returned %s", boolstr(bSuccess));

    if ( bSuccess )
    {
        ok("There's currently %d driver%s hooked", nbBytesReturned, PLURAL_IF(nbBytesReturned > 1));
    }

    return bSuccess;
}

bool
toggle_monitoring(HANDLE hFile, std::string const& arg, bool enable)
{
    std::wstring driver_name    = s2ws(arg);
    const usize driver_name_len = driver_name.length() * 2;
    const usize msglen          = std::min(driver_name_len, (usize)CFB_DRIVER_MAX_PATH);

    DWORD nbBytesReturned = 0;
    bool bSuccess         = ::DeviceIoControl(
        hFile,
        IOCTL_ControlDriver,
        driver_name.data(),
        msglen,
        nullptr,
        0,
        &nbBytesReturned,
        nullptr);
    info("toggle_monitoring(%s) returned %s", boolstr(enable), boolstr(bSuccess));

    return bSuccess;
}


bool
send_test_data(std::string const& device_name, u32 ioctl)
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
    const usize sz        = 0x20;
    auto buffer_in        = std::make_unique<u8[]>(sz);
    ::memset(buffer_in.get(), 'A', sz);

    bool bSuccess = ::DeviceIoControl(hFile.get(), ioctl, buffer_in.get(), sz, nullptr, 0, &nbBytesReturned, nullptr);
    info("send_test_data() returned %s", boolstr(bSuccess));

    return bSuccess;
}


int
main(int argc, const char** argv)
{
    argparse::ArgumentParser program("DriverClient");

    const std::vector<std::string> valid_actions =
        {"hook", "hook-unhook", "unhook", "size", "set-capturing", "send-data", "set-event"};

    program.add_argument("--action")
        .default_value(std::string("hook"))
        .action(
            [&valid_actions](const std::string& value)
            {
                if ( std::find(valid_actions.begin(), valid_actions.end(), value) != valid_actions.end() )
                {
                    return value;
                }
                return std::string {"hook"};
            });

    program.add_argument("--driver").default_value(std::string("\\driver\\hevd"));
    program.add_argument("--device").default_value(std::string("\\\\.\\HackSysExtremeVulnerableDriver"));
    program.add_argument("--ioctl").scan<'i', int>().default_value(
        0x222003); // https://github.com/hacksysteam/HackSysExtremeVulnerableDriver/blob/master/Driver/HEVD/Windows/HackSysExtremeVulnerableDriver.h#L81
    program.add_argument("--enable").default_value(false).implicit_value(true);

    try
    {
        program.parse_args(argc, argv);
    }
    catch ( const std::runtime_error& err )
    {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        std::exit(1);
    }

    auto action      = program.get<std::string>("--action");
    auto driver_name = program.get<std::string>("--driver");
    auto device_name = program.get<std::string>("--device");
    auto ioctl       = program.get<int>("--ioctl");
    auto enable      = program.get<bool>("--enable");

    wil::unique_handle hEvent;

    info("Getting a handle to '%S'", CFB_DEVICE_PATH);
    wil::unique_handle hFile(
        ::CreateFileW(CFB_USER_DEVICE_PATH, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr));
    if ( !hFile )
    {
        err("Failed to open '%S'", CFB_DEVICE_NAME);
        return -1;
    }

    if ( action == "hook" )
    {
        hook_driver(hFile.get(), driver_name);
    }
    else if ( action == "unhook" )
    {
        unhook_driver(hFile.get(), driver_name);
    }
    else if ( action == "hook-unhook" )
    {
        hook_driver(hFile.get(), driver_name);
        unhook_driver(hFile.get(), driver_name);
    }
    else if ( action == "size" )
    {
        get_size(hFile.get());
    }
    else if ( action == "set-capturing" )
    {
        toggle_monitoring(hFile.get(), driver_name, enable);
    }
    else if ( action == "send-data" )
    {
        send_test_data(device_name, ioctl);
    }
    else if ( action == "set-event" )
    {
        hEvent = wil::unique_handle(set_notif_handle(hFile.get()));
    }

    return 0;
}
