#include "Utils.h"


/*++
Routine Description:

The logging function for the driver. This function displays the message only if in DEBUG mode.


Arguments:

	- lpFormatString is a format string of the message to print

	- ...args are the arguments to the format string


Return Value:

	Does not return any value.

--*/
VOID CfbDbgPrint(IN const WCHAR* lpFormatString, ...)
{
    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
        return;


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

Routine Description:

A simple hexdumping function.


Arguments:

	- Buffer is a pointer to the address to hexdump

	- Length is the size (in bytes) of Buffer


Return Value:

	Does not return any value.

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

Routine Description:

A convenience function to quickly retrieve the name of a device directly from the device object.


Arguments:

	- pDeviceObject is a pointer to the device object
	 
	- DeviceNameBuffer is a pointer to wide buffer to store the device object name

	- DeviceNameBufferSize is the size (in bytes) of DeviceNameBuffer


Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS GetDeviceNameFromDeviceObject( _In_ PVOID pDeviceObject, _Out_ WCHAR* DeviceNameBuffer, _In_ ULONG DeviceNameBufferSize )
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
		return Status;

	if ( DeviceNameBufferSize < (ULONG)(2*(pDeviceNameInfo->Name.Length+1)) )
		return STATUS_BUFFER_TOO_SMALL;

	RtlSecureZeroMemory( DeviceNameBuffer, DeviceNameBufferSize );
	RtlCopyMemory( DeviceNameBuffer, pDeviceNameInfo->Name.Buffer, pDeviceNameInfo->Name.Length );
	return STATUS_SUCCESS;
}