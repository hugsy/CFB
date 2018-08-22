#pragma once

#include "Common.h"
#include "../Common/common.h"
#include "Utils.h"
#include "Queue.h"



extern NTKERNELAPI NTSTATUS IoGetRequestorSessionId(PIRP Irp, PULONG pSessionId);
extern NTKERNELAPI HANDLE PsGetCurrentThreadId();
extern NTKERNELAPI HANDLE PsGetProcessId(PEPROCESS Process);


NTSTATUS GetDataFromIrp(IN PIRP Irp, IN PIO_STACK_LOCATION Stack, IN PVOID *Buffer);
NTSTATUS PreparePipeMessage( IN UINT32 Pid, IN UINT32 Tid, IN UINT32 IoctlCode, IN PVOID pBody, IN ULONG BodyLen, OUT PSNIFFED_DATA *pMessage );
NTSTATUS HandleInterceptedIrp(IN PIRP Irp, IN PIO_STACK_LOCATION Stack);

VOID FreePipeMessage( IN PSNIFFED_DATA pMessage );