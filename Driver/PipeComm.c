#include "PipeComm.h"


/*++

Reference:
https://www.codeproject.com/Articles/9504/Driver-Development-Part-1-Introduction-to-Drivers
https://www.codeproject.com/Articles/8651/A-simple-demo-for-WDM-Driver-development

--*/
static NTSTATUS ExtractIrpData(IN PIRP Irp, IN ULONG Method, IN ULONG InputBufferLength, OUT PVOID *InputBuffer)
{
	NTSTATUS Status;

    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);

	PVOID Buffer = ExAllocatePoolWithTag(NonPagedPool, InputBufferLength, CFB_DEVICE_TAG);
	if (!Buffer)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	__try
	{
		Status = STATUS_SUCCESS;

		do
		{
			if (Stack->MajorFunction == IRP_MJ_DEVICE_CONTROL && Method == METHOD_NEITHER)
			{
				if (Stack->Parameters.DeviceIoControl.Type3InputBuffer >= (PVOID)(1 << 16))
				{
					RtlCopyMemory(Buffer, Stack->Parameters.DeviceIoControl.Type3InputBuffer, InputBufferLength);
				}
				else
				{
					Status = STATUS_UNSUCCESSFUL;
				}

				break;
			}

			if (Method == METHOD_BUFFERED)
			{
				if (!Irp->AssociatedIrp.SystemBuffer)
				{
					Status = STATUS_INVALID_PARAMETER;
				}
				else
				{
					RtlCopyMemory(Buffer, Irp->AssociatedIrp.SystemBuffer, InputBufferLength );
				}

				break;
			}

            if(Method == METHOD_IN_DIRECT)
            {
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
				    RtlCopyMemory(Buffer, pDataAddr, InputBufferLength );
			    }
            }

		} while (0);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		Status = GetExceptionCode();
		CfbDbgPrintErr(L"GetDataFromIrp() - Exception Code: 0x%X\n", Status);
	}


	if (!NT_SUCCESS(Status))
	{
		CfbDbgPrintErr(L"GetDataFromIrp() - Freeing %p\n", Buffer);
		ExFreePoolWithTag(Buffer, CFB_DEVICE_TAG);
	}
	else
	{
		*InputBuffer = Buffer;
	}

	return Status;
}



NTSTATUS ExtractDeviceIoctlIrpData(IN PIRP Irp, OUT PVOID *InputBuffer, OUT PULONG InputBufferLength)
{
    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);

    *InputBufferLength = Stack->Parameters.DeviceIoControl.InputBufferLength;
    if( *InputBufferLength == 0 )
    {
        return STATUS_SUCCESS;
    }

    ULONG IoctlCode = Stack->Parameters.DeviceIoControl.IoControlCode;
    ULONG Method = METHOD_FROM_CTL_CODE(IoctlCode);

    return ExtractIrpData(Irp, Method, *InputBufferLength, InputBuffer);
}


/*++

Extract the IRP data from a READ or WRITE.

Ref: 
- https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-mj-write
- https://docs.microsoft.com/en-us/windows-hardware/drivers/kernel/irp-mj-read

--*/
NTSTATUS ExtractReadWriteIrpData(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, OUT PVOID *InputBuffer, OUT PULONG InputBufferLength)
{
    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);

    if(Stack->MajorFunction == IRP_MJ_READ)
        *InputBufferLength = Stack->Parameters.Read.Length;
    else
        *InputBufferLength = Stack->Parameters.Write.Length;

    if(*InputBufferLength == 0)
    {
        return STATUS_SUCCESS;
    }

    ULONG Method;

    if(DeviceObject->Flags & DO_BUFFERED_IO)
    {
        Method = METHOD_BUFFERED;
    }
    else if(DeviceObject->Flags & DO_DIRECT_IO)
    {
        Method = METHOD_IN_DIRECT;
    }
    else
    {
        return STATUS_UNSUCCESSFUL;
    }

    return ExtractIrpData(Irp, Method, *InputBufferLength, InputBuffer);
}



/*++

Move the message from the stack to kernel pool.

--*/
NTSTATUS PreparePipeMessage(IN PPIPE_MESSAGE pIn, OUT PSNIFFED_DATA *pMessage)
{
	NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

	*pMessage = (PSNIFFED_DATA)ExAllocatePoolWithTag( NonPagedPool, sizeof( SNIFFED_DATA ), CFB_DEVICE_TAG );
	if ( !*pMessage )
	{
		return Status;
	}

	PSNIFFED_DATA_HEADER pMsgHeader = (PSNIFFED_DATA_HEADER)ExAllocatePoolWithTag( NonPagedPool, sizeof( SNIFFED_DATA_HEADER ), CFB_DEVICE_TAG );
	if ( !pMsgHeader )
	{
		ExFreePoolWithTag( *pMessage, CFB_DEVICE_TAG );
		return Status;
	}

	RtlSecureZeroMemory( pMsgHeader, sizeof( SNIFFED_DATA_HEADER ) );

	size_t szDriverNameLength = wcslen( pIn->DriverName );
	szDriverNameLength=szDriverNameLength > MAX_PATH ? MAX_PATH : szDriverNameLength + 1;
	
	size_t szDeviceNameLength = wcslen( pIn->DeviceName );
	szDeviceNameLength = szDeviceNameLength > MAX_PATH ? MAX_PATH : szDeviceNameLength + 1;


	//
	// create and fill the header structure
	//

	KeQuerySystemTime( &pMsgHeader->TimeStamp );
	pMsgHeader->Pid = pIn->Pid;
	pMsgHeader->Tid = pIn->Tid;
	pMsgHeader->Type = pIn->Type;
	pMsgHeader->Irql = KeGetCurrentIrql();
	pMsgHeader->InputBufferLength = pIn->InputBufferLen;
	pMsgHeader->InputBufferLength = pIn->OutputBufferLen;
	pMsgHeader->IoctlCode = pIn->IoctlCode;


	wcscpy_s( pMsgHeader->DriverName, szDriverNameLength, pIn->DriverName );
	wcscpy_s( pMsgHeader->DeviceName, szDeviceNameLength, pIn->DeviceName );


	//
	// fill up the message structure
	//

	(*pMessage)->Header = pMsgHeader;
	(*pMessage)->Body = pIn->InputBuffer;


	Status = STATUS_SUCCESS;

	return Status;
}


/*++

--*/
VOID FreePipeMessage(IN PSNIFFED_DATA pMessage)
{
	ExFreePoolWithTag(pMessage->Header, CFB_DEVICE_TAG);
	if (pMessage->Body) // can be NULL if pMessage->Body == 0
		ExFreePoolWithTag(pMessage->Body, CFB_DEVICE_TAG);
	ExFreePoolWithTag(pMessage, CFB_DEVICE_TAG);
	pMessage = NULL;
	return;
}


/*++

HandleInterceptedIrp is called every time we intercept an IRP. It will extract the data
from the IRP packet (depending on its method), and build a SNIFFED_DATA packet that will
written back to the userland client.

--*/
NTSTATUS HandleInterceptedIrp(IN PHOOKED_DRIVER Driver, IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PSNIFFED_DATA pMessage = NULL;
	PIPE_MESSAGE p = { 0, };
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation( Irp );

	p.Pid = (UINT32)((ULONG_PTR)PsGetProcessId( PsGetCurrentProcess() ) & 0xffffffff);
	p.Tid = (UINT32)((ULONG_PTR)PsGetCurrentThreadId() & 0xffffffff);
	p.Type = (UINT32)Stack->MajorFunction;
    p.OutputBufferLen = Stack->Parameters.DeviceIoControl.OutputBufferLength;

	wcsncpy( p.DriverName, Driver->Name, wcslen( Driver->Name ) );
	Status = GetDeviceNameFromDeviceObject( pDeviceObject, p.DeviceName, MAX_PATH );

	if ( !NT_SUCCESS( Status ) )
	{
		CfbDbgPrintWarn( L"GetDeviceName() failed, Status=0x%#x... Using empty string\n", Status );
	}

	switch (p.Type)
	{
        case IRP_MJ_DEVICE_CONTROL:
		    p.IoctlCode = Stack->Parameters.DeviceIoControl.IoControlCode;
            Status = ExtractDeviceIoctlIrpData(Irp, &p.InputBuffer, &p.InputBufferLen);
            if(!NT_SUCCESS(Status))
            {
                CfbDbgPrintErr(L"ExtractDeviceIoctlIrpData() failed, Status=%#X\n", Status);
                return Status;
            }
            break;

        case IRP_MJ_READ:
        case IRP_MJ_WRITE:
            Status = ExtractReadWriteIrpData(pDeviceObject, Irp, &p.InputBuffer, &p.InputBufferLen);
            break;

        default:
            CfbDbgPrintErr(L"Incorrect IRP type %x\n", p.Type);
            return STATUS_UNSUCCESSFUL;
	}


	Status = PreparePipeMessage(&p, &pMessage);

	if (!NT_SUCCESS(Status) || pMessage == NULL)
	{
		CfbDbgPrintErr(L"PreparePipeMessage() failed, Status=%#X\n", Status);
		if ( p.InputBuffer)
			ExFreePoolWithTag(p.InputBuffer, CFB_DEVICE_TAG);
		return Status;
	}


	//
	// push the new message with dumped IRP to the queue
	//
	UINT32 dwIndex;
	Status = PushToQueue( pMessage, &dwIndex );

	if ( !NT_SUCCESS( Status ) )
	{
		CfbDbgPrintErr( L"PushToQueue(%x) failed, discarding message\n", Status );
		FreePipeMessage( pMessage );
	}
	else
	{
		CfbDbgPrintOk( L"Message #%d (%dB) pushed to queue...\n", dwIndex, pMessage->Header->InputBufferLength);
	}

    
	//
	// and last, notify the client in UM of the new message posted
	//
	NotifyClient();

	return Status;
}