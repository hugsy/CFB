#pragma once

#include "Common.hpp"

namespace CFB::Log
{
#ifndef CFB_KERNEL_DRIVER
static inline const ULONG LogLevelDebug = 0;
static inline const ULONG LogLevelInfo  = 1;
#endif // CFB_KERNEL_DRIVER

void
Log(ULONG level, const char* fmtstr, ...);

#ifndef CFB_KERNEL_DRIVER
void
perror(const char* msg);

void
ntperror(const char* msg, const NTSTATUS Status);
#endif
}; // namespace CFB::Log

#ifndef CFB_NS
/// @brief Customizable log prefix
#define CFB_NS
#endif // !CFB_NS

#ifdef CFB_KERNEL_DRIVER

#ifdef _DEBUG
#define dbg(fmt, ...) CFB::Log::Log(DPFLTR_INFO_LEVEL, "[=] " CFB_NS " " fmt "\n", __VA_ARGS__)
#else
// #define dbg(fmt, ...)
#define dbg(fmt, ...) CFB::Log::log(DPFLTR_INFO_LEVEL, "[=] " CFB_NS " " fmt "\n", __VA_ARGS__)
#endif // _DEBUG

#define ok(fmt, ...) CFB::Log::Log(DPFLTR_INFO_LEVEL, "[+] " CFB_NS " " fmt "\n", __VA_ARGS__)
#define info(fmt, ...) CFB::Log::Log(DPFLTR_TRACE_LEVEL, "[*] " CFB_NS " " fmt "\n", __VA_ARGS__)
#define warn(fmt, ...) CFB::Log::Log(DPFLTR_WARNING_LEVEL, "[!] " CFB_NS " " fmt "\n", __VA_ARGS__)
#define err(fmt, ...) CFB::Log::Log(DPFLTR_ERROR_LEVEL, "[-] " CFB_NS " " fmt "\n", __VA_ARGS__)

#define DML(cmd) "<?dml?> <exec cmd=\"" cmd "\">" cmd "</exec>"

#else // CFB_KERNEL_DRIVER

#ifdef _DEBUG
#define dbg(fmt, ...) CFB::Log::Log(CFB::Log::LogLevelDebug, "[=] " CFB_NS " " fmt "\n", __VA_ARGS__)
#else
#define dbg(fmt, ...)
#endif // _DEBUG

#define ok(fmt, ...) CFB::Log::Log(CFB::Log::LogLevelInfo, "[+] " CFB_NS " " fmt "\n", __VA_ARGS__)
#define info(fmt, ...) CFB::Log::Log(CFB::Log::LogLevelInfo, "[*] " CFB_NS " " fmt "\n", __VA_ARGS__)
#define warn(fmt, ...) CFB::Log::Log(CFB::Log::LogLevelInfo, "[!] " CFB_NS " " fmt "\n", __VA_ARGS__)
#define err(fmt, ...) CFB::Log::Log(CFB::Log::LogLevelInfo, "[-] " CFB_NS " " fmt "\n", __VA_ARGS__)

#endif // CFB_KERNEL_DRIVER
