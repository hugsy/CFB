#include "PipeComm.h"

/*++

Reference:
https://www.codeproject.com/Articles/9504/Driver-Development-Part-1-Introduction-to-Drivers

--*/
NTSTATUS GetDataFromIrp(IN PIRP Irp, IN PIO_STACK_LOCATION Stack, IN PVOID *Buffer2)
{
	NTSTATUS Status = STATUS_SUCCESS;

	*Buffer2 = NULL;

	ULONG InputBufferLen = Stack->Parameters.DeviceIoControl.InputBufferLength;
	PVOID Buffer = ExAllocatePoolWithTag(NonPagedPool, InputBufferLen, CFB_DEVICE_TAG);
	if (!Buffer)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	__try
	{
		Status = STATUS_SUCCESS;

		do
		{
			ULONG IoctlCode = Stack->Parameters.DeviceIoControl.IoControlCode;
			if (METHOD_FROM_CTL_CODE(IoctlCode) == METHOD_NEITHER)
			{
				CfbDbgPrint(L"Using METHOD_NEITHER for IoctlCode=%#x\n", IoctlCode);
				if (Stack->Parameters.DeviceIoControl.Type3InputBuffer >= (PVOID)(1 << 16))
				{
					RtlCopyMemory(Buffer, Stack->Parameters.DeviceIoControl.Type3InputBuffer, InputBufferLen);
				}
				else
				{
					Status = STATUS_UNSUCCESSFUL;
				}

				break;
			}

			if (METHOD_FROM_CTL_CODE(IoctlCode) == METHOD_BUFFERED)
			{
				CfbDbgPrint(L"Using METHOD_BUFFERED for IoctlCode=%#x\n", IoctlCode);
				if (!Irp->AssociatedIrp.SystemBuffer)
				{
					Status = STATUS_INVALID_PARAMETER;
				}
				else
				{
					RtlCopyMemory(Buffer, Irp->AssociatedIrp.SystemBuffer, InputBufferLen);
				}

				break;
			}

			CfbDbgPrint(L"Using METHOD_IN_DIRECT for IoctlCode=%#x\n", IoctlCode);
			if (!Irp->MdlAddress)
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}

			PVOID pDataAddr = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
			if (!pDataAddr)
			{
				Status = STATUS_INVALID_PARAMETER;
				break;
			}
			else
			{
				RtlCopyMemory(Buffer, pDataAddr, InputBufferLen);
			}

		} while (0);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		Status = GetExceptionCode();
		CfbDbgPrint(L"[-] Exception Code: 0x%X\n", Status);
	}

	CfbDbgPrint(L"GetDataFromIrp() = %#x\n", Status);

	if (!NT_SUCCESS(Status))
	{
		CfbDbgPrint(L"[-] Freeing %p\n", Buffer);
		ExFreePoolWithTag(Buffer, CFB_DEVICE_TAG);
	}
	else
	{
		*Buffer2 = Buffer;
	}

	return Status;
}


/*++


--*/
NTSTATUS PreparePipeMessage(IN UINT32 Pid, IN UINT32 Tid, IN UINT32 IoctlCode, IN PVOID pBody, IN ULONG BodyLen, OUT PSNIFFED_DATA *pMessage)
{
	NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

	*pMessage = (PSNIFFED_DATA)ExAllocatePoolWithTag( NonPagedPool, sizeof( SNIFFED_DATA ), CFB_DEVICE_TAG );
	if ( !pMessage )
	{
		return Status;
	}

	PSNIFFED_DATA_HEADER pMsgHeader = (PSNIFFED_DATA_HEADER)ExAllocatePoolWithTag( NonPagedPool, sizeof( SNIFFED_DATA_HEADER ), CFB_DEVICE_TAG );
	if ( !pMsgHeader )
	{
		return Status;
	}


	//
	// create and fill the header structure
	//
	CfbDbgPrintInfo(L"Filling header %p\n", pMsgHeader);

	KeQuerySystemTime( &pMsgHeader->TimeStamp );
	pMsgHeader->Pid = Pid;
	pMsgHeader->Tid = Tid;
	//pMsgHeader->SessionId = Sid;
	pMsgHeader->Irql = KeGetCurrentIrql();
	pMsgHeader->BufferLength = BodyLen;
	pMsgHeader->IoctlCode = IoctlCode;


	//
	// fill up the message structure
	//
	CfbDbgPrintInfo( L"Filling message %p\n", *pMessage);

	(*pMessage)->Header = pMsgHeader;
	(*pMessage)->Body = pBody;

	Status = STATUS_SUCCESS;

	return Status;
}


/*++

--*/
VOID FreePipeMessage(PSNIFFED_DATA pMessage)
{
	ExFreePoolWithTag(pMessage->Header, CFB_DEVICE_TAG);
	ExFreePoolWithTag(pMessage->Body, CFB_DEVICE_TAG);
	ExFreePoolWithTag(pMessage, CFB_DEVICE_TAG);
	pMessage = NULL;
	return;
}


/*++

HandleInterceptedIrp is called every time we intercept an IRP. It will extract the data
from the IRP packet (depending on its method), and build a SNIFFED_DATA packet that will
be sent to the CFB_PIPE_NAME named pipe.

The fuzzer client (in userland) will read for new event on this pipe and start the fuzzing
jobs.

--*/
NTSTATUS HandleInterceptedIrp(IN PIRP Irp, IN PIO_STACK_LOCATION Stack)
{
	PAGED_CODE();

	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	UINT32 Pid, Tid = 0;
	//ULONG Sid = 0;
	PVOID IrpExtractedData;
	ULONG IrpExtractedDataLength = 0;
	UINT32 IoctlCode = 0;
	PSNIFFED_DATA pMessage = NULL;


	Status = GetDataFromIrp(Irp, Stack, &IrpExtractedData);

	if (!NT_SUCCESS(Status) || IrpExtractedData == NULL)
	{
		CfbDbgPrint(L"[-] GetDataFromIrp() failed, Status=%#X\n", Status);
		return Status;
	}


	IrpExtractedDataLength = Stack->Parameters.DeviceIoControl.InputBufferLength;

	CfbDbgPrint(L"Extracted IRP data (%p, %d B), dumping...\n", IrpExtractedData, IrpExtractedDataLength);
	if(IrpExtractedDataLength)
	{
		CfbHexDump( IrpExtractedData, IrpExtractedDataLength );
	}
	

	IoctlCode = Stack->Parameters.DeviceIoControl.IoControlCode;
	Pid = (UINT32)((ULONG_PTR)PsGetProcessId( PsGetCurrentProcess() ) & 0xffffffff);
	Tid =(UINT32)((ULONG_PTR)PsGetCurrentThreadId() & 0xffffffff);
	//IoGetRequestorSessionId(Irp, &Sid);

	Status = PreparePipeMessage(Pid, Tid, IoctlCode, IrpExtractedData, IrpExtractedDataLength, &pMessage);

	if (!NT_SUCCESS(Status) || pMessage == NULL)
	{
		CfbDbgPrintErr(L"PreparePipeMessage() failed, Status=%#X\n", Status);
		ExFreePoolWithTag(IrpExtractedData, CFB_DEVICE_TAG);
		return Status;
	}

	CfbDbgPrintOk(
		L"Prepared new pipe message %p (Pid=%llu, Ioctl=0x%#llx, Size=%lu)\n",
		pMessage,
		pMessage->Header->Pid,
		pMessage->Header->IoctlCode,
		pMessage->Header->BufferLength
	);

	Status = PushToQueue( pMessage );

	if ( !NT_SUCCESS( Status ) )
	{
		CfbDbgPrintErr( L"PushToQueue() failed, discarding message\n" );
		FreePipeMessage( pMessage );
	}

	CfbDbgPrintOk( L"Success! Message pushed to queue...\n" );

	return Status;
}