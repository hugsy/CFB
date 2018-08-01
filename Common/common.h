#pragma once

#ifndef _DRIVER
#include <Windows.h>
#include <stdio.h>
#endif // !_DRIVER



#define CFB_PROGRAM_NAME			L"Canadian Fuzzy Bear"
#define CFB_PROGRAM_NAME_SHORT		L"CFB"
#define CFB_AUTHOR					L"@_hugsy_"
#define CFB_VERSION					0.01

#define CFB_USER_DEVICE_NAME		L"\\\\.\\CFB"
#define CFB_DEVICE_NAME				L"\\Device\\CFB"
#define CFB_DEVICE_LINK				L"\\??\\CFB"



#ifdef _DEBUG
/* Debug */
#define WIDE2(x) L##x
#define WIDECHAR(x) WIDE2(x)

#define WIDE_FUNCTION WIDECHAR(__FUNCTION__) L"()"
#define WIDE_FILE WIDECHAR(__FILE__) 

#define dbg wprintf
#define GEN_FMT L"in '%s'(%s:%d) "
#define __xlog(t, ...) _xlog(t, __VA_ARGS__)
#define xlog(t, _f, ...) __xlog(t, GEN_FMT _f, WIDE_FUNCTION, WIDE_FILE, __LINE__, __VA_ARGS__)

#else 
/* Release */

#define dbg wprintf
#define xlog(t, ...) _xlog(t, __VA_ARGS__)

#endif /* _DEBUG_ */


typedef enum 
{
	LOG_DEBUG,
	LOG_INFO,
	LOG_SUCCESS,
	LOG_WARNING,
	LOG_ERROR,
	LOG_CRITICAL
} log_level_t;


__declspec(dllexport) void _xlog(log_level_t level, const wchar_t* format, ...);
__declspec(dllexport) void Hexdump(PVOID data, SIZE_T size);