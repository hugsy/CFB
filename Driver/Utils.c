#include "Utils.h"


/*++

--*/
VOID CfbDbgPrint(const WCHAR* lpFormatString, ...)
{
#ifdef _DEBUG
	va_list args;
	WCHAR buffer[1024] = { 0, };
	va_start(args, lpFormatString);
	vswprintf_s(buffer, sizeof(buffer) / sizeof(WCHAR), lpFormatString, args);
	va_end(args);

	// todo add timestamp
	KdPrint(("[CFB] %S", buffer));
#else
	UNREFERENCED_PARAMETER( lpFormatString );
#endif
}


/*++

Simple hexdumping function.

--*/
VOID CfbHexDump(PUCHAR Buffer, ULONG Length)
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