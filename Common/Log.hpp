#pragma once

#include "Common.hpp"

namespace CFB::Log
{
    void
    log(const char *lpFormatString, ...);

#ifndef CFB_KERNEL_DRIVER
    void
    perror(const char *msg);
#endif

    void
    ntperror(const char *msg, const NTSTATUS Status);
}; // namespace Log

#ifdef _DEBUG
#define dbg(fmt, ...) CFB::Log::log("[=] " fmt "\n", __VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif // _DEBUG

#define ok(fmt, ...) CFB::Log::log("[+] " fmt "\n", __VA_ARGS__)
#define info(fmt, ...) CFB::Log::log("[*] " fmt "\n", __VA_ARGS__)
#define warn(fmt, ...) CFB::Log::log("[!] " fmt "\n", __VA_ARGS__)
#define err(fmt, ...) CFB::Log::log("[-] " fmt "\n", __VA_ARGS__)
