#include <windows.h>
#include <wil/resource.h>

#include <string>

#include "Common.hpp"
#include "Log.hpp"
#include "IoctlCodes.hpp"


int wmain(int argc, const wchar_t* argv[])
{
    info("getting a handle to '%S'", CFB_DEVICE_PATH);

    std::wstring DriverToHook(L"tcpip.sys");
    DWORD nbBytesReturned;

    wil::unique_handle hFile(::CreateFileW(
        CFB_USER_DEVICE_PATH,
        GENERIC_WRITE | GENERIC_READ,
        0,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr)
    );
    if(!hFile)
    {
        err("Failed to open '%S'", CFB_DEVICE_NAME);
        return -1;
    }

    bool bSuccess = ::DeviceIoControl(
        hFile.get(),
        IOCTL_HookDriver,
        DriverToHook.data(),
        DriverToHook.size(),
        nullptr,
        0,
        &nbBytesReturned,
        nullptr
    );
    info("HookDriver() returned %s\n", boolstr(bSuccess));

    if (bSuccess)
    {
        bSuccess = ::DeviceIoControl(
            hFile.get(),
            IOCTL_UnhookDriver,
            DriverToHook.data(),
            DriverToHook.size(),
            nullptr,
            0,
            &nbBytesReturned,
            nullptr
        );
        info("UnhookDriver() returned %s\n", boolstr(bSuccess));
    }

    return 0;
}
