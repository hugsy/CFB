#include "Common.h"

#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>

#include "../common/Common.h"

#pragma once

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN(__FUNCTION__)

VOID CfbDbgLogInit();
VOID CfbDbgLogFree();
VOID CfbDbgPrint( IN const WCHAR* lpFormatString, ... );
VOID CfbHexDump( IN PUCHAR Buffer, IN ULONG Length );

#define CfbDbgPrintOk(fmt, ...)			CfbDbgPrint(L"[+] " __WFILE__ L":" __WFUNCTION__ L"() " fmt,  __VA_ARGS__)
#define CfbDbgPrintInfo(fmt, ...)		CfbDbgPrint(L"[*] " __WFILE__ L":" __WFUNCTION__ L"() " fmt,  __VA_ARGS__)
#define CfbDbgPrintErr(fmt, ...)		CfbDbgPrint(L"[-] " __WFILE__ L":" __WFUNCTION__ L"() " fmt,  __VA_ARGS__)
#define CfbDbgPrintWarn(fmt, ...)		CfbDbgPrint(L"[!] " __WFILE__ L":" __WFUNCTION__ L"() " fmt,  __VA_ARGS__)
#define CfbAssertIrqlMinLevel(Irql)		NT_ASSERT( KeGetCurrentIrql() > Irql )

extern NTKERNELAPI NTSTATUS ObQueryNameString(
	PVOID                    Object,
	POBJECT_NAME_INFORMATION ObjectNameInfo,
	ULONG                    Length,
	PULONG                   ReturnLength
);
//extern NTKERNELAPI NTSTATUS PsLookupProcessByProcessId(HANDLE ProcessId, PEPROCESS* Process);
extern NTKERNELAPI PSTR PsGetProcessImageFileName(IN PEPROCESS Process);

NTSTATUS GetDeviceNameFromDeviceObject( _In_ PVOID pDeviceObject, _Out_ WCHAR* DeviceNameBuffer, _In_ ULONG DeviceNameBufferSize );
NTSTATUS GetProcessNameFromPid(IN UINT32 Pid, OUT PUNICODE_STRING *StrDst);