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
NTSTATUS PreparePipeMessage(IN ULONG Pid, IN ULONG Sid, IN ULONG IoctlCode, IN PVOID pBody, IN ULONG BodyLen, OUT PSNIFFED_DATA *pMessage)
{
	NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

	PSNIFFED_DATA pMsgOutput=(PSNIFFED_DATA)ExAllocatePoolWithTag( NonPagedPool, sizeof( SNIFFED_DATA ), CFB_DEVICE_TAG );
	if ( !pMsgOutput )
	{
		return Status;
	}

	PSNIFFED_DATA_HEADER pMsgHeader=(PSNIFFED_DATA_HEADER)ExAllocatePoolWithTag( NonPagedPool, sizeof( SNIFFED_DATA_HEADER ), CFB_DEVICE_TAG );
	if ( !pMsgHeader )
	{
		ExFreePoolWithTag( pMsgOutput, CFB_DEVICE_TAG );
		return Status;
	}


	//
	// create and fill the header structure
	//
	CfbDbgPrint(L"Filling header %p\n", pMsgHeader);

	KeQuerySystemTime( &pMsgHeader->TimeStamp );
	pMsgHeader->Pid = Pid;
	pMsgHeader->SessionId = Sid;
	pMsgHeader->Irql = KeGetCurrentIrql();
	pMsgHeader->BufferLength = BodyLen;
	pMsgHeader->IoctlCode = IoctlCode;


	//
	// fill up the message structure
	//
	CfbDbgPrint( L"Filling message %p\n", pMsgOutput);

	pMsgOutput->Header = pMsgHeader;
	pMsgOutput->Body = pBody;

	Status = STATUS_SUCCESS;

	*pMessage = pMsgOutput;

	return Status;
}


/*++

From http://www.osronline.com/showThread.cfm?link=71108

--*/
NTSTATUS SendToPipe(IN PSNIFFED_DATA pData)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	HANDLE hPipe;
	PWSTR lpPipeName = CFB_PIPE_NAME;
	OBJECT_ATTRIBUTES ObjectAttributes = { 0, };
	UNICODE_STRING UnicodePipeName;
	IO_STATUS_BLOCK IoStatusBlock;

	ULONG Flags = OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE | OBJ_FORCE_ACCESS_CHECK;
	ULONG CreateOptions = /*FILE_NON_DIRECTORY_FILE |*/ FILE_SYNCHRONOUS_IO_NONALERT;

	RtlInitUnicodeString(
		&UnicodePipeName,
		lpPipeName
	);

	InitializeObjectAttributes(
		&ObjectAttributes,
		&UnicodePipeName,
		Flags,
		NULL,
		NULL
	);

	__try
	{

		Status = ZwCreateFile(
			&hPipe,
			SYNCHRONIZE | GENERIC_WRITE,
			&ObjectAttributes,
			&IoStatusBlock,
			NULL,
			FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_WRITE,
			FILE_OPEN_IF,
			CreateOptions,
			NULL,
			0
		);

		if (!NT_SUCCESS(Status))
		{
			CfbDbgPrintErr(L"ZwCreateFile('%s') failed: Status=0x%X\n", lpPipeName, Status);
		}
		else
		{

			//
			// Send the header
			//
			Status = ZwWriteFile(
				hPipe,
				NULL,
				NULL,
				NULL,
				&IoStatusBlock,
				pData->Header,
				sizeof(SNIFFED_DATA_HEADER),
				NULL,
				NULL
			);

			if (!NT_SUCCESS(Status))
			{
				CfbDbgPrint(L"ZwWriteFile('%s', header) failed: Status=0x%X\n", lpPipeName, Status);
			}
			else
			{

				//
				// Send the body
				//
				Status = ZwWriteFile(
					hPipe,
					NULL,
					NULL,
					NULL,
					&IoStatusBlock,
					pData->Body,
					pData->Header->BufferLength,
					NULL,
					NULL
				);

				if (!NT_SUCCESS(Status))
				{
					CfbDbgPrint( L"ZwWriteFile('%s', body) failed: Status=0x%X\n", lpPipeName, Status );
				}

			}

			ZwClose(hPipe);
		}

	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		Status = GetExceptionCode();
		CfbDbgPrintErr(L"Exception Code: 0x%X\n", Status);
	}
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
	ULONG Pid, Sid = 0;
	PVOID IrpExtractedData;
	ULONG IrpExtractedDataLength = 0;
	ULONG IoctlCode = 0;
	PSNIFFED_DATA pMessage;


	Status = GetDataFromIrp(Irp, Stack, &IrpExtractedData);

	if (!NT_SUCCESS(Status) || IrpExtractedData == NULL)
	{
		CfbDbgPrint(L"[-] GetDataFromIrp() failed, Status=%#X\n", Status);
		return Status;
	}


	IrpExtractedDataLength = Stack->Parameters.DeviceIoControl.InputBufferLength;

	CfbDbgPrint(L"Extracted IRP data (%p, %d B), dumping...\n", IrpExtractedData, IrpExtractedDataLength);
	CfbHexDump(IrpExtractedData, IrpExtractedDataLength);


	IoctlCode = Stack->Parameters.DeviceIoControl.IoControlCode;
	Pid = IoGetRequestorProcessId(Irp);
	IoGetRequestorSessionId(Irp, &Sid);

	Status = PreparePipeMessage(Pid, Sid, IoctlCode, IrpExtractedData, IrpExtractedDataLength, &pMessage);

	if (!NT_SUCCESS(Status) || pMessage == NULL)
	{
		CfbDbgPrintErr(L"PreparePipeMessage() failed, Status=%#X\n", Status);
		ExFreePoolWithTag(IrpExtractedData, CFB_DEVICE_TAG);
		return Status;
	}

	CfbDbgPrintOk(
		L"Prepared new pipe message %p (Pid=%d, Ioctl=%#x, Size=%d)\n",
		pMessage->Header->Pid,
		pMessage->Header->IoctlCode,
		pMessage->Header->BufferLength
	);

	Status = SendToPipe(pMessage);

	if (!NT_SUCCESS(Status))
	{
		CfbDbgPrintErr(L"SendToPipe() failed\n");
	}

	FreePipeMessage(pMessage);

	return Status;
}