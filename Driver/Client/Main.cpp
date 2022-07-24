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

#define PLURAL_IF(x) ((x) ? "s" : "")

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
hook_driver(HANDLE hFile, std::string const& arg)
{

    DWORD nbBytesReturned       = 0;
    IoMessage msg               = {0};
    std::wstring driver_name    = s2ws(arg);
    const usize driver_name_len = driver_name.length() * 2;
    const usize msglen          = std::min(driver_name_len, (usize)CFB_DRIVER_MAX_PATH);

    ::RtlCopyMemory(msg.DriverName, driver_name.data(), msglen);

    bool bSuccess = ::DeviceIoControl(hFile, IOCTL_HookDriver, &msg, msglen, nullptr, 0, &nbBytesReturned, nullptr);
    info("HookDriver() returned %s", boolstr(bSuccess));

    if ( !bSuccess )
        return false;

    return true;
}

bool
unhook_driver(HANDLE hFile, std::string const& arg)
{

    DWORD nbBytesReturned       = 0;
    IoMessage msg               = {0};
    std::wstring driver_name    = s2ws(arg);
    const usize driver_name_len = driver_name.length() * 2;
    const usize msglen          = std::min(driver_name_len, (usize)CFB_DRIVER_MAX_PATH);

    ::RtlCopyMemory(msg.DriverName, driver_name.data(), msglen);

    bool bSuccess = ::DeviceIoControl(hFile, IOCTL_UnhookDriver, &msg, msglen, nullptr, 0, &nbBytesReturned, nullptr);
    info("UnhookDriver() returned %s", boolstr(bSuccess));

    if ( !bSuccess )
        return false;

    return true;
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
toggle_monitoring(HANDLE hFile, std::vector<std::string> const& driver_names, bool state)
{
    return true;
}


int
main(int argc, const char** argv)
{
    argparse::ArgumentParser program("DriverClient");

    program.add_argument("--action")
        .default_value(std::string("hook"))
        .action(
            [](const std::string& value)
            {
                static const std::vector<std::string> choices = {"hook", "hook-unhook", "unhook", "size"};
                if ( std::find(choices.begin(), choices.end(), value) != choices.end() )
                {
                    return value;
                }
                return std::string {"hook"};
            });
    program.add_argument("--driver").default_value(std::string("\\driver\\tcpip")).required();

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

    return 0;
}
