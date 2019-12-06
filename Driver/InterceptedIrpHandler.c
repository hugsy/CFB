#include "InterceptedIrpHandler.h"

typedef struct
{
	UINT32 Pid;
	UINT32 Tid;
	UINT32 IoctlCode;
	UINT32 Type;
	WCHAR DriverName[MAX_PATH];
	WCHAR DeviceName[MAX_PATH];
	PVOID InputBuffer;
	PVOID OutputBuffer;
	ULONG InputBufferLen;
	ULONG OutputBufferLen;
}
HOOKED_IRP_INFO, *PHOOKED_IRP_INFO;




/*++

Reference:
https://www.codeproject.com/Articles/9504/Driver-Development-Part-1-Introduction-to-Drivers
https://www.codeproject.com/Articles/8651/A-simple-demo-for-WDM-Driver-development

--*/
static NTSTATUS ExtractIrpData(IN PIRP Irp, IN ULONG Method, IN ULONG BufferLength, OUT PVOID *OutBuffer)
{
	PVOID Buffer = ExAllocatePoolWithTag(PagedPool, BufferLength, CFB_DEVICE_TAG);
	if (!Buffer)
		return STATUS_INSUFFICIENT_RESOURCES;

	RtlSecureZeroMemory(Buffer, BufferLength);

    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);

	__try
	{
		do
		{
			if ((Stack->MajorFunction == IRP_MJ_DEVICE_CONTROL || Stack->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL) && \
				Method == METHOD_NEITHER)
			{
				if (Stack->Parameters.DeviceIoControl.Type3InputBuffer >= (PVOID)(1 << 16))
					RtlCopyMemory(Buffer, Stack->Parameters.DeviceIoControl.Type3InputBuffer, BufferLength);
				else
					Status = STATUS_INVALID_PARAMETER;
				break;
			}

			if (Method == METHOD_BUFFERED)
			{
				if (Irp->AssociatedIrp.SystemBuffer)
					RtlCopyMemory(Buffer, Irp->AssociatedIrp.SystemBuffer, BufferLength);
				else
					Status = STATUS_INVALID_PARAMETER_1;
				break;
			}

            if(Method == METHOD_IN_DIRECT || Method == METHOD_OUT_DIRECT)
            {
			    if (!Irp->MdlAddress)
			    {
				    Status = STATUS_INVALID_PARAMETER_2;
				    break;
			    }

			    PVOID pDataAddr = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
			    if (!pDataAddr)
			    {
				    Status = STATUS_INVALID_PARAMETER_3;
				    break;
			    }

			    RtlCopyMemory(Buffer, pDataAddr, BufferLength );
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
		*OutBuffer = Buffer;
	}

	return Status;
}



NTSTATUS ExtractDeviceIoctlIrpData(IN PIRP Irp, OUT PVOID *Buffer, OUT PULONG BufferLength)
{
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);

    *BufferLength = Stack->Parameters.DeviceIoControl.InputBufferLength;
    if( *BufferLength == 0 )
        return STATUS_SUCCESS;

    ULONG IoctlCode = Stack->Parameters.DeviceIoControl.IoControlCode;
    ULONG Method = METHOD_FROM_CTL_CODE(IoctlCode);

    return ExtractIrpData(Irp, Method, *BufferLength, Buffer);
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

	if (Stack->MajorFunction == IRP_MJ_READ)
	{
		*InputBufferLength = Stack->Parameters.Read.Length;
		return STATUS_SUCCESS;
	}

    *InputBufferLength = Stack->Parameters.Write.Length;

    if(*InputBufferLength == 0)
        return STATUS_SUCCESS;


    ULONG Method;

    if(DeviceObject->Flags & DO_BUFFERED_IO)
        Method = METHOD_BUFFERED;

    else if(DeviceObject->Flags & DO_DIRECT_IO)
        Method = METHOD_IN_DIRECT;

    else
        return STATUS_UNSUCCESSFUL;

    return ExtractIrpData(Irp, Method, *InputBufferLength, InputBuffer);
}



/*++

Move the message from the stack to kernel pool.

--*/
NTSTATUS PreparePipeMessage(IN PHOOKED_IRP_INFO pIn, OUT PINTERCEPTED_IRP *pIrp)
{
	*pIrp = (PINTERCEPTED_IRP)ExAllocatePoolWithTag( PagedPool, sizeof(INTERCEPTED_IRP), CFB_DEVICE_TAG );
	if ( !*pIrp)
		return STATUS_INSUFFICIENT_RESOURCES;

	RtlSecureZeroMemory(*pIrp, sizeof(INTERCEPTED_IRP));

	//
	// Allocate the intercepted IRP header...
	//
	PINTERCEPTED_IRP_HEADER pIrpHeader = (PINTERCEPTED_IRP_HEADER)ExAllocatePoolWithTag( 
        PagedPool,
		sizeof( INTERCEPTED_IRP_HEADER ), 
		CFB_DEVICE_TAG 
	);

	if (!pIrpHeader)
	{
		ExFreePoolWithTag( *pIrp, CFB_DEVICE_TAG );
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RtlSecureZeroMemory(pIrpHeader, sizeof(INTERCEPTED_IRP_HEADER) );

	size_t szDriverNameLength = wcslen( pIn->DriverName );
	szDriverNameLength = szDriverNameLength > MAX_PATH ? MAX_PATH : szDriverNameLength + 1;
	
	size_t szDeviceNameLength = wcslen( pIn->DeviceName );
	szDeviceNameLength = szDeviceNameLength > MAX_PATH ? MAX_PATH : szDeviceNameLength + 1;

	PWCHAR lpswProcessName = L"unknown\0";
	size_t szProcessNameLength = 8;
	UNICODE_STRING u = { 0 };
	PUNICODE_STRING us = &u;

	if (NT_SUCCESS(GetProcessNameFromPid(pIn->Pid, &us)) && us->Length)
	{
		lpswProcessName = us->Buffer;
		szProcessNameLength = us->Length < MAX_PATH ? us->Length : MAX_PATH;
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
	wcscpy_s(pIrpHeader->ProcessName, szProcessNameLength, lpswProcessName);

	if (us)
		RtlFreeUnicodeString(us);

	//
	// fill up the message structure
	//

	(*pIrp)->Header = pIrpHeader;
	(*pIrp)->InputBuffer = pIn->InputBuffer;

	return STATUS_SUCCESS;
}


/*++

Totally wipe all traces of the IRP.

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
from the IRP packet (depending on its method), and build a INTERCEPTED_IRP object that will
written back to the userland client.

--*/
NTSTATUS HandleInterceptedIrp(IN PHOOKED_DRIVER Driver, IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp, OUT PINTERCEPTED_IRP *pIrpOut)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PINTERCEPTED_IRP pIrp = NULL;
	HOOKED_IRP_INFO temp = { 0, };
	PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation( Irp );
	
	temp.Pid = HandleToULong(PsGetProcessId(PsGetCurrentProcess()));
	temp.Tid = HandleToULong(PsGetCurrentThreadId());
	temp.Type = (CFB_INTERCEPTED_IRP_TYPE_IRP |(UINT32)Stack->MajorFunction);

	wcsncpy( temp.DriverName, Driver->Name, wcslen( Driver->Name ) );

	Status = GetDeviceNameFromDeviceObject( pDeviceObject, temp.DeviceName, MAX_PATH );
	if ( !NT_SUCCESS( Status ) )
		CfbDbgPrintWarn( L"Cannot get device name, using empty string (Status=0x%#x)\n", Status );


	switch (temp.Type)
	{
        case IRP_MJ_DEVICE_CONTROL:
        case IRP_MJ_INTERNAL_DEVICE_CONTROL:
			temp.OutputBufferLen = Stack->Parameters.DeviceIoControl.OutputBufferLength;
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

        default:
            return STATUS_NOT_IMPLEMENTED;
	}


	Status = PreparePipeMessage(&temp, &pIrp);

	if (!NT_SUCCESS(Status) || pIrp == NULL)
	{
		CfbDbgPrintErr(L"PreparePipeMessage() failed, Status=%#X\n", Status);
		if (temp.InputBuffer)
		{
			ExFreePoolWithTag(temp.InputBuffer, CFB_DEVICE_TAG);
			temp.InputBuffer = NULL;
		}
			
		return Status;
	}

	*pIrpOut = pIrp;

	return STATUS_SUCCESS;
}


/*++

This function is called when a synchronous IRP is done being processed, so we can
collect additional info about the result.

--*/
NTSTATUS 
CompleteHandleInterceptedIrp(
	_In_ PIO_STACK_LOCATION Stack, 
	_In_ PVOID UserBuffer, 
	_In_ NTSTATUS IrpStatus, 
	_Inout_ PINTERCEPTED_IRP pIrpInfo
)
{
	//
	// Complete the info
	//
	pIrpInfo->Header->Status = IrpStatus;


	//
	// For read and ioctls, update the OutputBufferLength
	//
	switch (pIrpInfo->Header->Type)
	{
	case IRP_MJ_READ:
		pIrpInfo->Header->OutputBufferLength = Stack->Parameters.Read.Length;
		break;

	case IRP_MJ_DEVICE_CONTROL:
	case IRP_MJ_INTERNAL_DEVICE_CONTROL:
		pIrpInfo->Header->OutputBufferLength = Stack->Parameters.DeviceIoControl.OutputBufferLength;
		break;

	default:
		//
		// for any other type we don't care about the output, just return
		//
		return STATUS_SUCCESS;
	}


	//
	// check the INTERCEPTED_IRP consistency 
	//
	if (UserBuffer == NULL)
	{
		pIrpInfo->OutputBuffer = NULL;

		if (pIrpInfo->Header->OutputBufferLength > 0)
		{
			pIrpInfo->OutputBuffer = NULL;
			return STATUS_INVALID_PARAMETER_2;
		}

		return STATUS_SUCCESS;
	}


	pIrpInfo->OutputBuffer = ExAllocatePoolWithTag(
		PagedPool, 
		pIrpInfo->Header->OutputBufferLength, 
		CFB_DEVICE_TAG
	);
	if (!pIrpInfo->OutputBuffer)
		return STATUS_INSUFFICIENT_RESOURCES;

	RtlSecureZeroMemory(pIrpInfo->OutputBuffer, pIrpInfo->Header->OutputBufferLength);

	RtlCopyMemory(pIrpInfo->OutputBuffer, UserBuffer, pIrpInfo->Header->OutputBufferLength);

	return STATUS_SUCCESS;
}



NTSTATUS 
HandleInterceptedFastIo(
	_In_ PHOOKED_DRIVER Driver, 
	_In_ PDEVICE_OBJECT pDeviceObject, 
	_In_ UINT32 Type,
	_In_ UINT32 IoctlCode,
	_In_ PVOID Buffer,
	_In_ ULONG BufferLength,
	_In_ UINT32 Flags,
	_Inout_ PINTERCEPTED_IRP* pIrpOut
)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PINTERCEPTED_IRP pIrp = NULL;
	HOOKED_IRP_INFO temp = { 0, };

	BOOLEAN IsInput = Flags & CFB_FASTIO_USE_INPUT_BUFFER;


	//
	// prepare the metadata
	//
	temp.Pid = HandleToULong(PsGetProcessId(PsGetCurrentProcess()));
	temp.Tid = HandleToULong(PsGetCurrentThreadId());
	temp.Type = Type;
	temp.IoctlCode = IoctlCode;

	wcsncpy(temp.DriverName, Driver->Name, wcslen(Driver->Name));

	Status = GetDeviceNameFromDeviceObject(pDeviceObject, temp.DeviceName, MAX_PATH);
	if (!NT_SUCCESS(Status))
		CfbDbgPrintWarn(L"Cannot get device name, using empty string (Status=0x%#x)\n", Status);


	//
	// Copy the input buffer
	//
	if (IsInput)
	{
		if (BufferLength && Buffer)
		{
			temp.InputBuffer = ExAllocatePoolWithTag(
				PagedPool,
				BufferLength,
				CFB_DEVICE_TAG
			);
			if (!temp.InputBuffer)
				return STATUS_INSUFFICIENT_RESOURCES;

			RtlSecureZeroMemory(temp.InputBuffer, BufferLength);

			RtlCopyMemory(temp.InputBuffer, Buffer, BufferLength);

			temp.InputBufferLen = BufferLength;
		}
		else
		{
			temp.InputBufferLen = 0;
			temp.InputBuffer = NULL;
		}
	}
	else
	{
		if (BufferLength && Buffer)
		{
			temp.OutputBuffer = ExAllocatePoolWithTag(
				PagedPool,
				BufferLength,
				CFB_DEVICE_TAG
			);
			if (!temp.OutputBuffer)
				return STATUS_INSUFFICIENT_RESOURCES;

			RtlSecureZeroMemory(temp.OutputBuffer, BufferLength);

			RtlCopyMemory(temp.OutputBuffer, Buffer, BufferLength);

			temp.OutputBufferLen = BufferLength;
		}
		else
		{
			temp.OutputBufferLen = 0;
			temp.OutputBuffer = NULL;
		}
	}



	if (Flags & CFB_FASTIO_INIT_QUEUE_MESSAGE)
	{
		//
		// Prepare the message to be queued
		//
		Status = PreparePipeMessage(&temp, &pIrp);

		if (!NT_SUCCESS(Status) || pIrp == NULL)
		{
			CfbDbgPrintErr(L"PreparePipeMessage() failed, Status=%#X\n", Status);
			if (IsInput && temp.InputBuffer)
			{
				ExFreePoolWithTag(temp.InputBuffer, CFB_DEVICE_TAG);
				temp.InputBuffer = NULL;
			}
			else if (!IsInput && temp.OutputBuffer)
			{
				ExFreePoolWithTag(temp.OutputBuffer, CFB_DEVICE_TAG);
				temp.OutputBuffer = NULL;
			}

			return Status;
		}
	}

	*pIrpOut = pIrp;
	return Status;
}