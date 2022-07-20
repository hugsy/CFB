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

#ifdef CFB_KERNEL_DRIVER
#ifdef _DEBUG
#define CFB_LOG_VERBOSE_LEVEL DPFLTR_TRACE_LEVEL
#else
#define CFB_LOG_VERBOSE_LEVEL DPFLTR_WARNING_LEVEL
#endif
#endif

namespace CFB::Log
{
void
log(const char* fmtstr, ...)
{
    va_list args;
    va_start(args, fmtstr);
    char buffer[1024] = {0};

#ifdef CFB_KERNEL_DRIVER
    RtlStringCchVPrintfA(buffer, countof(buffer), fmtstr, args);
    DbgPrintEx(DPFLTR_IHVDRIVER_ID, CFB_LOG_VERBOSE_LEVEL, buffer);
#else
    ::vsprintf_s(buffer, countof(buffer), fmtstr, args);
    std::cout << buffer;
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
