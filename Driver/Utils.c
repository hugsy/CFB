#include "Utils.h"


/*++

--*/
VOID CfbDbgPrint(IN const WCHAR* lpFormatString, ...)
{
#ifdef _DEBUG
	va_list args;
	WCHAR buffer[1024] = { 0, };
	va_start(args, lpFormatString);
	vswprintf_s(buffer, sizeof(buffer) / sizeof(WCHAR), lpFormatString, args);
	va_end(args);

	KdPrint(("[%S] %S", CFB_PROGRAM_NAME_SHORT, buffer));
#else
	UNREFERENCED_PARAMETER( lpFormatString );
#endif
}


/*++

Simple hexdumping function.

--*/
VOID CfbHexDump(IN PUCHAR Buffer, IN ULONG Length)
{
#ifdef _DEBUG
	for (ULONG i = 0; i < Length; i++)
	{
		if(i%16==0)	KdPrint(("\n"));
		KdPrint(("%02x ", (UCHAR)Buffer[i]));
	}
	KdPrint(("\n"));
#else
	UNREFERENCED_PARAMETER( Buffer );
	UNREFERENCED_PARAMETER( Length );
#endif
}


/*++

Convenience function to retrieve the name of a device directly from the device object.

--*/
NTSTATUS GetDeviceNameFromDeviceObject( IN PVOID pDeviceObject, OUT WCHAR* DeviceNameBuffer, IN ULONG DeviceNameBufferSize )
{
	NTSTATUS Status;
	UCHAR Buffer[0x400] = { 0, };
	ULONG ReturnLength;
	POBJECT_NAME_INFORMATION pDeviceNameInfo = (POBJECT_NAME_INFORMATION)Buffer;

	Status = ObQueryNameString(
								pDeviceObject,
								pDeviceNameInfo,
								sizeof( Buffer ),
								&ReturnLength
	);

	if ( !NT_SUCCESS( Status ) )
	{
		CfbDbgPrintErr( L"ObQueryNameString() failed: Status=%x\n", Status );
		return Status;
	}

	if ( DeviceNameBufferSize < (ULONG)(2*(pDeviceNameInfo->Name.Length+1)) )
	{
		CfbDbgPrintErr( L"Buffer is too small\n" );
		return STATUS_BUFFER_TOO_SMALL;
	}

	RtlSecureZeroMemory( DeviceNameBuffer, DeviceNameBufferSize );

	RtlCopyMemory( DeviceNameBuffer, pDeviceNameInfo->Name.Buffer, pDeviceNameInfo->Name.Length );

	return STATUS_SUCCESS;
}