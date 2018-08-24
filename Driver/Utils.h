#ifndef __UTILS_H__
#define __UTILS_H__

#include "Common.h"

#include <stdio.h>
#include <wchar.h>
#include <stdarg.h>

#pragma once

VOID CfbDbgPrint(const WCHAR* lpFormatString, ...);
VOID CfbHexDump(UCHAR *Buffer, ULONG Length);

#define CfbDbgPrintErr(fmt, ...)  CfbDbgPrint(L"[-] " fmt,  __VA_ARGS__)
#define CfbDbgPrintOk(fmt, ...)   CfbDbgPrint(L"[+] " fmt,  __VA_ARGS__)
#define CfbDbgPrintInfo(fmt, ...)   CfbDbgPrint(L"[*] " fmt,  __VA_ARGS__)
#define CfbDbgPrintWarn(fmt, ...)   CfbDbgPrint(L"[!] " fmt,  __VA_ARGS__)

extern NTKERNELAPI NTSTATUS ObQueryNameString(
	PVOID                    Object,
	POBJECT_NAME_INFORMATION ObjectNameInfo,
	ULONG                    Length,
	PULONG                   ReturnLength
);


NTSTATUS GetDeviceNameFromDeviceObject( IN PVOID pDeviceObject, OUT WCHAR* DeviceNameBuffer, IN ULONG DeviceNameBufferSize );

#endif /* __UTILS_H__ */
