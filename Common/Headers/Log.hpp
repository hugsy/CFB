#pragma once

#include "Common.hpp"

namespace CFB::Log
{
void
log(
#ifdef CFB_KERNEL_DRIVER
    ULONG level,
#endif // CFB_KERNEL_DRIVER
    const char* fmtstr,
    ...);

#ifndef CFB_KERNEL_DRIVER
void
perror(const char* msg);

void
ntperror(const char* msg, const NTSTATUS Status);
#endif
}; // namespace CFB::Log

#ifdef CFB_KERNEL_DRIVER

#ifdef _DEBUG
#define dbg(fmt, ...) CFB::Log::log(DPFLTR_INFO_LEVEL, "[=] " fmt "\n", __VA_ARGS__)
#else
// #define dbg(fmt, ...)
#define dbg(fmt, ...) CFB::Log::log(DPFLTR_INFO_LEVEL, "[=] " fmt "\n", __VA_ARGS__)
#endif // _DEBUG

#define ok(fmt, ...) CFB::Log::log(DPFLTR_INFO_LEVEL, "[+] " fmt "\n", __VA_ARGS__)
#define info(fmt, ...) CFB::Log::log(DPFLTR_TRACE_LEVEL, "[*] " fmt "\n", __VA_ARGS__)
#define warn(fmt, ...) CFB::Log::log(DPFLTR_WARNING_LEVEL, "[!] " fmt "\n", __VA_ARGS__)
#define err(fmt, ...) CFB::Log::log(DPFLTR_ERROR_LEVEL, "[-] " fmt "\n", __VA_ARGS__)

#define DML(cmd) "<?dml?> <exec cmd=\"" cmd "\">" cmd "</exec>"
#else

#ifdef _DEBUG
#define dbg(fmt, ...) CFB::Log::log("[=] " fmt "\n", __VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif // _DEBUG

#define ok(fmt, ...) CFB::Log::log("[+] " fmt "\n", __VA_ARGS__)
#define info(fmt, ...) CFB::Log::log("[*] " fmt "\n", __VA_ARGS__)
#define warn(fmt, ...) CFB::Log::log("[!] " fmt "\n", __VA_ARGS__)
#define err(fmt, ...) CFB::Log::log("[-] " fmt "\n", __VA_ARGS__)

#endif
