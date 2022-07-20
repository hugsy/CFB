#include <wil/resource.h>
#include <windows.h>

#include <string>

#include "Common.hpp"
#include "IoctlCodes.hpp"
#include "Log.hpp"
#include "Messages.hpp"


int
wmain(int argc, const wchar_t* argv[])
{
    info("getting a handle to '%S'", CFB_DEVICE_PATH);

    std::wstring DriverToHook(L"\\driver\\http");
    DWORD nbBytesReturned;

    IoMessage msg      = {0};
    const usize msglen = min(DriverToHook.length() * 2, CFB_DRIVER_MAX_PATH);

    ::RtlCopyMemory(msg.DriverName, DriverToHook.data(), msglen);

    wil::unique_handle hFile(
        ::CreateFileW(CFB_USER_DEVICE_PATH, GENERIC_WRITE | GENERIC_READ, 0, nullptr, OPEN_EXISTING, 0, nullptr));
    if ( !hFile )
    {
        err("Failed to open '%S'", CFB_DEVICE_NAME);
        return -1;
    }

    bool bSuccess =
        ::DeviceIoControl(hFile.get(), IOCTL_HookDriver, &msg, msglen, nullptr, 0, &nbBytesReturned, nullptr);
    info("HookDriver() returned %s", boolstr(bSuccess));

    if ( !bSuccess )
        return -1;

    bSuccess = ::DeviceIoControl(hFile.get(), IOCTL_UnhookDriver, &msg, msglen, nullptr, 0, &nbBytesReturned, nullptr);

    info("UnhookDriver() returned %s", boolstr(bSuccess));
    if ( !bSuccess )
        return -1;


    return 0;
}
