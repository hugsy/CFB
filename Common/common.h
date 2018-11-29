#pragma once

#ifndef _DRIVER
#include <Windows.h>
#include <stdio.h>
#endif // !_DRIVER



#define CFB_PROGRAM_NAME			L"Canadian Fuzzy Bear"
#define CFB_PROGRAM_NAME_SHORT		L"CFB"
#define CFB_AUTHOR					L"@_hugsy_"
#define CFB_VERSION					0.01

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


#ifdef _DEBUG
/* Debug */

#define WIDE2(x) L##x
#define WIDECHAR(x) WIDE2(x)

#define WIDE_FUNCTION WIDECHAR(__FUNCTION__) L"()"
#define WIDE_FILE WIDECHAR(__FILE__)

#define GEN_FMT L"in '%s'(%s:%d) [%d] "
#define __xlog(t, ...) _xlog(t, __VA_ARGS__)
#define xlog(t, _f, ...) __xlog(t, GEN_FMT _f, WIDE_FUNCTION, WIDE_FILE, __LINE__, GetThreadId(GetCurrentThread()), __VA_ARGS__)

#else
/* Release */

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


typedef struct 
{
	BOOLEAN Enabled;
	WCHAR Name[MAX_PATH];
}
HOOKED_DRIVER_INFO, *PHOOKED_DRIVER_INFO;

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
}
INTERCEPTED_IRP_HEADER, *PINTERCEPTED_IRP_HEADER;
# pragma pack ()

typedef PVOID PINTERCEPTED_IRP_BODY;

typedef struct 
{
	PINTERCEPTED_IRP_HEADER Header;
	PINTERCEPTED_IRP_BODY RawBuffer;
	LIST_ENTRY ListEntry;
}
INTERCEPTED_IRP, *PINTERCEPTED_IRP;


__declspec(dllexport) void _xlog(log_level_t level, const wchar_t* format, ...);
__declspec(dllexport) void Hexdump(PVOID data, SIZE_T size);