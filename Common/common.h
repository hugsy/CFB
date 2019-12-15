#pragma once

#ifndef _DRIVER
#include <Windows.h>
#include <stdio.h>
#endif

#define CFB_PROGRAM_NAME			L"Canadian Fuzzy Bear"
#define CFB_PROGRAM_NAME_SHORT		L"CFB"
#define CFB_AUTHOR					L"@_hugsy_"
#define CFB_VERSION					0.2

#define CFB_USER_DEVICE_NAME		L"\\\\.\\IrpDumper"
#define CFB_DEVICE_NAME				L"\\Device\\IrpDumper"
#define CFB_DEVICE_LINK				L"\\DosDevices\\IrpDumper"

#define CFB_DRIVER_NAME				L"IrpDumper.sys"
#define CFB_SERVICE_NAME			L"IrpDumper"
#define CFB_SERVICE_DESCRIPTION		L"CFB IRP Dumper Driver"

#ifndef MAX_PATH
#define MAX_PATH                    0x104
#endif
#define HOOKED_DRIVER_MAX_NAME_LEN	MAX_PATH

#define CFB_PIPE_NAME               L"\\\\.\\pipe\\CFB"
#define CFB_PIPE_MAXCLIENTS			PIPE_UNLIMITED_INSTANCES
#define CFB_PIPE_INBUFLEN			4096
#define CFB_PIPE_OUTBUFLEN			4096


#define WIDE2(x) L##x
#define WIDECHAR(x) WIDE2(x)

#define FUNCTIONW WIDECHAR(__FUNCTION__) L"()"
#define FILENAMEW WIDECHAR(__FILE__)


#ifdef _DEBUG

/* Debug */
#define GEN_FMT L"in '%s'(%s:%d) [%d] "
#define __xlog(t, ...) _xlog(t, __VA_ARGS__)
#define xlog(t, _f, ...) __xlog(t, GEN_FMT _f, FUNCTIONW, FILENAMEW, __LINE__, GetThreadId(GetCurrentThread()), __VA_ARGS__)

#define dbg(...) _xlog(LOG_DEBUG, __VA_ARGS__)

#else

/* Release */
#define xlog(t, ...) _xlog(t, __VA_ARGS__)
#define dbg(...) 

#endif /* _DEBUG_ */


#ifdef _COLORIZE
#define COLOR_RESET L"\033[0m"
#define COLOR_BOLD L"\033[1m"
#define COLOR_UNDERLINE L"\033[4m"

#define COLOR_FG_BLACK L"\033[30m"
#define COLOR_FG_RED L"\033[31m"
#define COLOR_FG_GREEN L"\033[32m"
#define COLOR_FG_YELLOW L"\033[33m"
#define COLOR_FG_BLUE L"\033[34m"
#define COLOR_FG_MAGENTA L"\033[35m"
#define COLOR_FG_CYAN L"\033[36m"
#define COLOR_FG_WHITE L"\033[37m"
#else
#define COLOR_RESET
#define COLOR_BOLD
#define COLOR_UNDERLINE 

#define COLOR_FG_BLACK
#define COLOR_FG_RED
#define COLOR_FG_GREEN
#define COLOR_FG_YELLOW 
#define COLOR_FG_BLUE
#define COLOR_FG_MAGENTA
#define COLOR_FG_CYAN
#define COLOR_FG_WHITE
#endif

#define PrintErrorWithFunctionName(x) PrintError(FILENAMEW L":" FUNCTIONW L": " x)


typedef enum
{
	LOG_DEBUG,
	LOG_INFO,
	LOG_SUCCESS,
	LOG_WARNING,
	LOG_ERROR,
	LOG_CRITICAL
} log_level_t;

# pragma pack (1)
typedef struct 
{
	UINT32 Enabled;
	WCHAR Name[MAX_PATH];
	UINT32 NumberOfDevices;
	UINT64 NumberOfRequestIntercepted;
	ULONG_PTR DriverAddress;
}
HOOKED_DRIVER_INFO, *PHOOKED_DRIVER_INFO;
# pragma pack ()

# pragma pack (1)
typedef struct
{
	LARGE_INTEGER TimeStamp;
	UINT32 Irql;
	UINT32 Type;
	UINT32 IoctlCode;
	UINT32 Pid;
	UINT32 Tid;
	UINT32 InputBufferLength;
    UINT32 OutputBufferLength;
	WCHAR DriverName[MAX_PATH];
	WCHAR DeviceName[MAX_PATH];
	WCHAR ProcessName[MAX_PATH];
	NTSTATUS Status;
}
INTERCEPTED_IRP_HEADER, *PINTERCEPTED_IRP_HEADER;
# pragma pack ()

typedef PVOID PINTERCEPTED_IRP_BODY;

typedef struct 
{
	PINTERCEPTED_IRP_HEADER Header;
	PINTERCEPTED_IRP_BODY InputBuffer;
	PINTERCEPTED_IRP_BODY OutputBuffer;
	LIST_ENTRY ListEntry;
}
INTERCEPTED_IRP, *PINTERCEPTED_IRP;


__declspec(dllexport) void hexdump(PVOID data, SIZE_T size);
__declspec(dllexport) void PrintError(const wchar_t* msg);
__declspec(dllexport) void _xlog(log_level_t level, const wchar_t* format, ...);
__declspec(dllexport) char* CreateRandomString(const size_t len);
__declspec(dllexport) wchar_t* CreateRandomWideString(const size_t len);
__declspec(dllexport) void GenerateRandomString(char* str, const size_t len);
