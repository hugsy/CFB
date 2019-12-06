#include "Utils.h"

#define CFB_DRIVER_LOG_BUFSIZE 4096

static PWCHAR g_LogBuffer = NULL;
static FAST_MUTEX g_LogMutex;


VOID CfbDbgLogInit()
{
	if (!g_LogBuffer)
	{
		g_LogBuffer = (PWCHAR)ExAllocatePoolWithTag(NonPagedPool, CFB_DRIVER_LOG_BUFSIZE, CFB_DEVICE_TAG);
		if (!g_LogBuffer)
			return;
		ExInitializeFastMutex(&g_LogMutex);
	}
}


VOID CfbDbgLogFree()
{
	if (g_LogBuffer)
	{
		ExFreePoolWithTag(g_LogBuffer, CFB_DEVICE_TAG);
	}
}


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

#ifdef _DEBUG
	va_list args;

	ExAcquireFastMutex(&g_LogMutex);
	RtlZeroMemory(g_LogBuffer, CFB_DRIVER_LOG_BUFSIZE);

	va_start(args, lpFormatString);
	vswprintf_s(g_LogBuffer, CFB_DRIVER_LOG_BUFSIZE / sizeof(WCHAR), lpFormatString, args);
	va_end(args);

	KdPrint(("[%S] %S", CFB_PROGRAM_NAME_SHORT, g_LogBuffer));
	ExReleaseFastMutex(&g_LogMutex);

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
		if(i%16==0 && i)	KdPrint(("\n"));
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



/*++

Routine Description:

A convenience function to the name as a UNICODE_STRING from its PID.


Arguments:

	- Pid is the process ID

	- StrDst is a pointer to the UNICODE_STRING receiving the result if successful

Return Value:

	Returns STATUS_SUCCESS on success.

--*/
NTSTATUS GetProcessNameFromPid(IN UINT32 Pid, OUT PUNICODE_STRING *StrDst)
{
	PEPROCESS Process;
	
	if (NT_SUCCESS(PsLookupProcessByProcessId(UlongToHandle(Pid), &Process)))
	{
		PSTR lpProcessName = PsGetProcessImageFileName(Process);
		if (lpProcessName)
		{
			CANSI_STRING as = { 0, };
			if (NT_SUCCESS(RtlInitAnsiStringEx(&as, lpProcessName)))
				return RtlAnsiStringToUnicodeString(*StrDst, &as, TRUE);
		}
	}

	return STATUS_UNSUCCESSFUL;
}