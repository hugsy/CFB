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

VOID CfbDbgPrint( IN const WCHAR* lpFormatString, ... );
VOID CfbHexDump( IN PUCHAR Buffer, IN ULONG Length );

#define CfbDbgPrintErr(fmt, ...)		CfbDbgPrint(L"[-] " __WFILE__ L":" __WFUNCTION__ L"() " fmt,  __VA_ARGS__)
#define CfbDbgPrintOk(fmt, ...)			CfbDbgPrint(L"[+] " __WFILE__ L":" __WFUNCTION__ L"() " fmt,  __VA_ARGS__)
#define CfbDbgPrintInfo(fmt, ...)		CfbDbgPrint(L"[*] " __WFILE__ L":" __WFUNCTION__ L"() " fmt,  __VA_ARGS__)
#define CfbDbgPrintWarn(fmt, ...)		CfbDbgPrint(L"[!] " __WFILE__ L":" __WFUNCTION__ L"() " fmt,  __VA_ARGS__)
#define CfbAssertIrqlMinLevel(Irql)		NT_ASSERT( KeGetCurrentIrql() > Irql )

extern NTKERNELAPI NTSTATUS ObQueryNameString(
	PVOID                    Object,
	POBJECT_NAME_INFORMATION ObjectNameInfo,
	ULONG                    Length,
	PULONG                   ReturnLength
);


NTSTATUS GetDeviceNameFromDeviceObject( IN PVOID pDeviceObject, OUT WCHAR* DeviceNameBuffer, IN ULONG DeviceNameBufferSize );


