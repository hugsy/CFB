#include "Log.hpp"

#ifdef CFB_KERNEL_DRIVER
#include <ntstrsafe.h>

#else
#include <stdarg.h>
#include <stdio.h>

#include <iostream>
#endif // CFB_KERNEL_DRIVER

#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN(__FUNCTION__)


namespace CFB::Log
{
void
Log(ULONG level, const char* fmtstr, ...)
{
    va_list args;
    va_start(args, fmtstr);

#ifdef CFB_KERNEL_DRIVER
    //
    // Use `nt!Kd_IHVDRIVER_Mask` to control the level
    //
    ::vDbgPrintEx(DPFLTR_IHVDRIVER_ID, level, fmtstr, args);

#else
    std::string out;
    out.resize(1024);

    ::vsnprintf(out.data(), out.size(), fmtstr, args);

    if ( level == LogLevelDebug )
    {
        ::OutputDebugStringA(out.c_str());
    }
    else
    {
        out.resize(std::strlen(out.c_str()));
        std::cerr << out;
    }
#endif // CFB_KERNEL_DRIVER

    va_end(args);
}

#ifndef CFB_KERNEL_DRIVER

extern "C" void WINAPI
SetLastError(_In_ DWORD dwErrCode);

extern "C" ULONG WINAPI
RtlNtStatusToDosError(_In_ NTSTATUS Status);


//
// User specific logging functions
//

void
perror(const char* msg)
{
    const u32 sysMsgSz = 2048;
    auto sysMsg        = std::string();
    sysMsg.resize(sysMsgSz);
    const auto errcode = ::GetLastError();

    ::FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_MAX_WIDTH_MASK, // FORMAT_MESSAGE_FROM_HMODULE
        nullptr,
        errcode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        sysMsg.data(),
        sysMsgSz,
        nullptr);

    const usize max_len = ::wcslen((wchar_t*)sysMsg.c_str());
    err("%s, errcode=0x%08x: %s", msg, errcode, sysMsg.c_str());
}

void
ntperror(const char* msg, const NTSTATUS Status)
{
    auto dwDosError = RtlNtStatusToDosError(Status);
    auto hResult    = HRESULT_FROM_WIN32(dwDosError);
    ::SetLastError(hResult);
    CFB::Log::perror(msg);
    return;
}
#endif
} // namespace CFB::Log
