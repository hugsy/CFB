#include "InterceptedIrpHandler.h"

///
/// This is an internal structure to PipeComm. Don't use elsewhere.
///
typedef struct
{
	UINT32 Pid;
	UINT32 Tid;
	UINT32 IoctlCode;
	UINT32 Type;
	WCHAR DriverName[MAX_PATH];
	WCHAR DeviceName[MAX_PATH];
	PVOID InputBuffer;
	ULONG InputBufferLen;
	ULONG OutputBufferLen;
}
HOOKED_IRP_INFO, *PHOOKED_IRP_INFO;




/*++

Reference:
https://www.codeproject.com/Articles/9504/Driver-Development-Part-1-Introduction-to-Drivers
https://www.codeproject.com/Articles/8651/A-simple-demo-for-WDM-Driver-development

--*/
static NTSTATUS ExtractIrpData(IN PIRP Irp, IN ULONG Method, IN ULONG InputBufferLength, OUT PVOID *InputBuffer)
{

	PVOID Buffer = ExAllocatePoolWithTag(NonPagedPool, InputBufferLength, CFB_DEVICE_TAG);
	if (!Buffer)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);

	__try
	{

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
		CfbDbgPrintErr(L"Exception Code: 0x%X\n", Status);
	}


	if (!NT_SUCCESS(Status))
	{
		CfbDbgPrintErr(L"Failed to copy data (Status=0x%x), freeing %p\n", Status, Buffer);
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
NTSTATUS PreparePipeMessage(IN PHOOKED_IRP_INFO pIn, OUT PINTERCEPTED_IRP *pIrp)
{
	NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;

	*pIrp = (PINTERCEPTED_IRP)ExAllocatePoolWithTag( NonPagedPool, sizeof( INTERCEPTED_IRP ), CFB_DEVICE_TAG );
	if ( !*pIrp)
	{
		return Status;
	}


	//
	// Allocate the intercepted IRP header...
	//
	PINTERCEPTED_IRP_HEADER pIrpHeader = (PINTERCEPTED_IRP_HEADER)ExAllocatePoolWithTag( 
        NonPagedPool,
		sizeof( INTERCEPTED_IRP_HEADER ), 
		CFB_DEVICE_TAG 
	);

	if ( !pIrpHeader)
	{
		ExFreePoolWithTag( *pIrp, CFB_DEVICE_TAG );
		return Status;
	}

	RtlSecureZeroMemory(pIrpHeader, sizeof(INTERCEPTED_IRP_HEADER) );

	size_t szDriverNameLength = wcslen( pIn->DriverName );
	szDriverNameLength = szDriverNameLength > MAX_PATH ? MAX_PATH : szDriverNameLength + 1;
	
	size_t szDeviceNameLength = wcslen( pIn->DeviceName );
	szDeviceNameLength = szDeviceNameLength > MAX_PATH ? MAX_PATH : szDeviceNameLength + 1;

	PEPROCESS Process;
	PWCHAR szProcessName = L"unknown\0";
	size_t szProcessNameLength = 8;
	if (PsLookupProcessByProcessId((HANDLE)pIn->Pid, &Process) != STATUS_INVALID_PARAMETER)
	{
		PSTR lpProcessName = PsGetProcessImageFileName(Process);
		if (lpProcessName)
		{
			UNICODE_STRING us = { 0, };
			CANSI_STRING as = { 0 };
			RtlInitAnsiStringEx(&as, lpProcessName);
			if (NT_SUCCESS(RtlAnsiStringToUnicodeString(&us, &as, FALSE)))
			{
				szProcessName = us.Buffer;
				szProcessNameLength = us.Length;
				szProcessNameLength = szProcessNameLength >= MAX_PATH ? MAX_PATH : szProcessNameLength;
			}
		}
	}


	//
	// ... and fill it up
	//

	KeQuerySystemTime( &pIrpHeader->TimeStamp );
	pIrpHeader->Pid = pIn->Pid;
	pIrpHeader->Tid = pIn->Tid;
	pIrpHeader->Type = pIn->Type;
	pIrpHeader->Irql = KeGetCurrentIrql();
	pIrpHeader->InputBufferLength = pIn->InputBufferLen;
	pIrpHeader->OutputBufferLength = pIn->OutputBufferLen;
	pIrpHeader->IoctlCode = pIn->IoctlCode;


	wcscpy_s(pIrpHeader->DriverName, szDriverNameLength, pIn->DriverName );
	wcscpy_s(pIrpHeader->DeviceName, szDeviceNameLength, pIn->DeviceName );

	wcscpy_s(pIrpHeader->ProcessName, szProcessNameLength, szProcessName);


	//
	// fill up the message structure
	//

	(*pIrp)->Header = pIrpHeader;
	(*pIrp)->InputBuffer = pIn->InputBuffer;

	return STATUS_SUCCESS;
}


/*++

--*/
VOID FreeInterceptedIrp(IN PINTERCEPTED_IRP pIrp)
{
    UINT32 dwBodyLen = pIrp->Header->InputBufferLength;
	if (pIrp->InputBuffer && dwBodyLen)
	{
        RtlSecureZeroMemory(pIrp->InputBuffer, dwBodyLen);
		ExFreePoolWithTag(pIrp->InputBuffer, CFB_DEVICE_TAG);
        pIrp->InputBuffer = NULL;
	}

	dwBodyLen = pIrp->Header->OutputBufferLength;
	if (pIrp->OutputBuffer && dwBodyLen)
	{
		RtlSecureZeroMemory(pIrp->OutputBuffer, dwBodyLen);
		ExFreePoolWithTag(pIrp->OutputBuffer, CFB_DEVICE_TAG);
		pIrp->OutputBuffer = NULL;
	}

    RtlSecureZeroMemory(pIrp->Header, sizeof(INTERCEPTED_IRP_HEADER));
    ExFreePoolWithTag(pIrp->Header, CFB_DEVICE_TAG);

    RtlSecureZeroMemory(pIrp, sizeof(INTERCEPTED_IRP));
	ExFreePoolWithTag(pIrp, CFB_DEVICE_TAG);
	pIrp = NULL;
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
	PINTERCEPTED_IRP pIrp = NULL;
	HOOKED_IRP_INFO temp = { 0, };
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation( Irp );
	
	temp.Pid = HandleToULong(PsGetProcessId(PsGetCurrentProcess()));
	temp.Tid = HandleToULong(PsGetCurrentThreadId());
	temp.Type = (UINT32)Stack->MajorFunction;
    temp.OutputBufferLen = Stack->Parameters.DeviceIoControl.OutputBufferLength;

	wcsncpy( temp.DriverName, Driver->Name, wcslen( Driver->Name ) );
	Status = GetDeviceNameFromDeviceObject( pDeviceObject, temp.DeviceName, MAX_PATH );

	if ( !NT_SUCCESS( Status ) )
	{
		CfbDbgPrintWarn( L"Cannot get device name, using empty string (Status=0x%#x)\n", Status );
	}

	switch (temp.Type)
	{
        case IRP_MJ_DEVICE_CONTROL:
        case IRP_MJ_INTERNAL_DEVICE_CONTROL:
		    temp.IoctlCode = Stack->Parameters.DeviceIoControl.IoControlCode;
            Status = ExtractDeviceIoctlIrpData(Irp, &temp.InputBuffer, &temp.InputBufferLen);
            if(!NT_SUCCESS(Status))
            {
                CfbDbgPrintErr(L"ExtractDeviceIoctlIrpData() failed, Status=%#X\n", Status);
                return Status;
            }
            break;

        case IRP_MJ_READ:
        case IRP_MJ_WRITE:
            Status = ExtractReadWriteIrpData(pDeviceObject, Irp, &temp.InputBuffer, &temp.InputBufferLen);
            break;

        //default:
        //    return STATUS_NOT_IMPLEMENTED;
	}


	Status = PreparePipeMessage(&temp, &pIrp);

	if (!NT_SUCCESS(Status) || pIrp == NULL)
	{
		CfbDbgPrintErr(L"PreparePipeMessage() failed, Status=%#X\n", Status);
		if (temp.InputBuffer)
		{
			ExFreePoolWithTag(temp.InputBuffer, CFB_DEVICE_TAG);
		}
			
		return Status;
	}


	//
	// push the new message with dumped IRP to the queue
	//
	Status = PushToQueue( pIrp );

	if ( !NT_SUCCESS( Status ) )
	{
		CfbDbgPrintErr( L"PushToQueue(%p) failed, discarding message = %x\n", pIrp, Status );
		FreeInterceptedIrp( pIrp );
	}
    else
    {
	    //
	    // and last, notify the client in UM of the new message posted
	    //

        SetNewIrpInQueueAlert();
        CfbDbgPrintOk(L"IRP %p queued (IrpQueueSize=%d)\n", pIrp, GetIrpListSize());
    }

	return Status;
}