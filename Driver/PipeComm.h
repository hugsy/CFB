#ifndef __PIPE_COMM_H__
#define __PIPE_COMM_H__

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"

#pragma once



typedef struct __sniffed_data_header_t
{
	LARGE_INTEGER TimeStamp;
	KIRQL Irql;
	ULONG IoctlCode;
	ULONG Pid;
	ULONG SessionId;
	ULONG BufferLength;
	WCHAR DriverName[HOOKED_DRIVER_MAX_NAME_LEN];
}
SNIFFED_DATA_HEADER, *PSNIFFED_DATA_HEADER;


typedef PVOID PSNIFFED_DATA_BODY;


typedef struct __sniffed_data_t
{
	PSNIFFED_DATA_HEADER Header;
	PSNIFFED_DATA_BODY Body;
}
SNIFFED_DATA, *PSNIFFED_DATA;


extern NTKERNELAPI NTSTATUS IoGetRequestorSessionId(PIRP Irp, PULONG pSessionId);
extern NTKERNELAPI ULONG IoGetRequestorProcessId(PIRP Irp);


NTSTATUS GetDataFromIrp(IN PIRP Irp, IN PIO_STACK_LOCATION Stack, IN PVOID *Buffer);
NTSTATUS PreparePipeMessage(IN ULONG Pid, IN ULONG Sid, IN ULONG IoctlCode, IN PVOID pBody, IN ULONG BodyLen, OUT PSNIFFED_DATA *pMsgOutput);
NTSTATUS SendToPipe(IN PSNIFFED_DATA pData);
NTSTATUS HandleInterceptedIrp(IN PIRP Irp, IN PIO_STACK_LOCATION Stack);

#endif /* __PIPE_COMM_H__ */