#include "Log.hpp"

#ifdef CFB_KERNEL_DRIVER
#include <ntstrsafe.h>

#else
#include <stdarg.h>
#include <stdio.h>

#include <iostream>

#endif //! CFB_KERNEL_DRIVER

#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN(__FUNCTION__)


namespace CFB::Log
{
void
log(const char* fmtstr, ...)
{
    va_list args;
    va_start(args, fmtstr);

#ifdef CFB_KERNEL_DRIVER
    //
    // Explicitly refusing to log anything if IRQ level too high, since most Rtl* encoding functions are for
    // PASSIVE_LEVEL only
    //
    if ( ::KeGetCurrentIrql() >= DISPATCH_LEVEL )
    {
        return;
    }

#ifdef _DEBUG
    ::vDbgPrintEx(DPFLTR_IHVDRIVER_ID, 0xFFFFFFFF, fmtstr, args);
#else
    ::vDbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_WARNING_LEVEL, fmtstr, args);
#endif // _DEBUG
#else
    ::vprintf(fmtstr, args);
#endif // CFB_KERNEL_DRIVER

    va_end(args);
}

#ifndef CFB_KERNEL_DRIVER
void
perror(const char* msg)
{
    const u32 sysMsgSz = 1024;
    auto sysMsg        = std::string();
    sysMsg.reserve(sysMsgSz);
    const auto errcode = ::GetLastError();

    ::FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
        nullptr,
        errcode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        sysMsg.data(),
        sysMsgSz,
        nullptr);

    const usize max_len = ::wcslen((wchar_t*)sysMsg.c_str());
    err("%s, errcode=0x%08x: %s", msg, errcode, sysMsg.c_str());
}
#endif

void
ntperror(const char* msg, const NTSTATUS Status)
{
    err("%s, Status=0x%08x", msg, Status);
    return;
}

} // namespace CFB::Log
