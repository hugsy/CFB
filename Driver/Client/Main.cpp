#define NOMINMAX
#undef _UNICODE
#undef UNICODE

// clang-format off
#include <windows.h>

#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include <argparse.hpp>
#include <wil/resource.h>

#include "Common.hpp"
#include "IoctlCodes.hpp"
#include "Log.hpp"
#include "Messages.hpp"
// clang-format on

#pragma warning(push)
#pragma warning(disable : 4244) // bad but only for tests
std::string
ws2s(std::wstring const& ws)
{
    return std::string(ws.begin(), ws.end());
}

std::wstring
s2ws(std::string const& s)
{
    return std::wstring(s.begin(), s.end());
}
#pragma warning(pop)

bool
hook_driver(std::vector<std::string> const& args)
{
    info("Getting a handle to '%S'", CFB_DEVICE_PATH);
    wil::unique_handle hFile(
        ::CreateFileW(CFB_USER_DEVICE_PATH, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr));
    if ( !hFile )
    {
        err("Failed to open '%S'", CFB_DEVICE_NAME);
        return false;
    }

    for ( auto const& arg : args )
    {
        DWORD nbBytesReturned       = 0;
        IoMessage msg               = {0};
        std::wstring driver_name    = s2ws(arg);
        const usize driver_name_len = driver_name.length() * 2;
        const usize msglen          = std::min(driver_name_len, (usize)CFB_DRIVER_MAX_PATH);

        ::RtlCopyMemory(msg.DriverName, driver_name.data(), msglen);

        bool bSuccess =
            ::DeviceIoControl(hFile.get(), IOCTL_HookDriver, &msg, msglen, nullptr, 0, &nbBytesReturned, nullptr);
        info("HookDriver() returned %s", boolstr(bSuccess));

        if ( !bSuccess )
            return false;
    }

    return true;
}

bool
unhook_driver(std::vector<std::string> const& args)
{
    info("Getting a handle to '%S'", CFB_DEVICE_PATH);
    wil::unique_handle hFile(
        ::CreateFileW(CFB_USER_DEVICE_PATH, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr));
    if ( !hFile )
    {
        err("Failed to open '%S'", CFB_DEVICE_NAME);
        return false;
    }

    for ( auto const& arg : args )
    {
        DWORD nbBytesReturned       = 0;
        IoMessage msg               = {0};
        std::wstring driver_name    = s2ws(arg);
        const usize driver_name_len = driver_name.length() * 2;
        const usize msglen          = std::min(driver_name_len, (usize)CFB_DRIVER_MAX_PATH);

        ::RtlCopyMemory(msg.DriverName, driver_name.data(), msglen);

        bool bSuccess =
            ::DeviceIoControl(hFile.get(), IOCTL_UnhookDriver, &msg, msglen, nullptr, 0, &nbBytesReturned, nullptr);
        info("UnhookDriver() returned %s", boolstr(bSuccess));

        if ( !bSuccess )
            return false;
    }

    return true;
}

bool
get_size()
{
    info("Getting a handle to '%S'", CFB_DEVICE_PATH);
    wil::unique_handle hFile(
        ::CreateFileW(CFB_USER_DEVICE_PATH, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr));
    if ( !hFile )
    {
        err("Failed to open '%S'", CFB_DEVICE_NAME);
        return false;
    }

    DWORD nbBytesReturned = 0;
    bool bSuccess =
        ::DeviceIoControl(hFile.get(), IOCTL_GetNumberOfDrivers, nullptr, 0, nullptr, 0, &nbBytesReturned, nullptr);
    info("GetNumberOfDrivers() returned %s", boolstr(bSuccess));

    if ( bSuccess )
    {
        ok("There's currently %d drivers hooked", nbBytesReturned);
    }

    return bSuccess;
}


int
main(int argc, const char** argv)
{
    // std::vector<std::string> args2;
    // std::copy(argv, argv + argc, std::back_inserter(args2));

    // for ( auto const& arg : args2 )
    // {
    //     std::wstring ws = s2ws(arg);
    //     std::string s   = ws2s(ws);

    //     std::wcout << L"warg = L'" << ws << L"'" << std::endl;
    //     std::cout << "arg = '" << s << "'" << std::endl;
    // }

    // return 0;

    argparse::ArgumentParser program("DriverClient");

    program.add_argument("--action").default_value(std::string("hook"));
    program.add_argument("args").remaining();

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

    auto action = program.get<std::string>("--action");
    auto args   = program.get<std::vector<std::string>>("args");

    if ( action == "hook" )
    {
        hook_driver(args);
    }
    else if ( action == "unhook" )
    {
        unhook_driver(args);
    }
    else if ( action == "hook-unhook" )
    {
        hook_driver(args);
        unhook_driver(args);
    }
    else if ( action == "size" )
    {
        get_size();
    }

    return 0;
}
