#include "stdafx.h"

#include "common.h"


static HANDLE g_hLogMutex = NULL;



/*++

--*/
void _xlog(log_level_t level, const wchar_t* format, ...)
{
#ifndef _DEBUG
	//
	// If we're not in Debug mode, we don't care about xlog(LOG_DEBUG) 
	//
	if (level == LOG_DEBUG)
		return;
#endif

	if (g_hLogMutex == NULL)
	{
		g_hLogMutex = CreateMutex(NULL, FALSE, NULL);
		if (g_hLogMutex == NULL)
			return;
	}

	const wchar_t* prio;

	switch (level)
	{
	case LOG_DEBUG:
		prio = L"[DEBUG] ";
		break;
	case LOG_INFO:
		prio =  L"[INFO] ";
		break;
	case LOG_SUCCESS:
		prio = COLOR_FG_GREEN L"[SUCCESS] " COLOR_RESET;
		break;
	case LOG_WARNING:
		prio = COLOR_FG_YELLOW L"[WARNING] " COLOR_RESET;
		break;
	case LOG_ERROR:
		prio = COLOR_BOLD COLOR_FG_RED L"[ERROR] " COLOR_RESET;
		break;
	case LOG_CRITICAL:
		prio = COLOR_BOLD COLOR_FG_MAGENTA L"[CRITICAL] " COLOR_RESET;
		break;
	default:
		return;
	}

	WaitForSingleObject(g_hLogMutex, INFINITE);

	va_list args;
	SYSTEMTIME lt;
	GetLocalTime(&lt);

#ifdef _DEBUG
	fwprintf(stderr, L"%02d-%02d-%02d %02d:%02d:%02d ",
		lt.wYear, lt.wMonth, lt.wDay,
		lt.wHour, lt.wMinute, lt.wSecond);
#endif

	fwprintf(stderr, L"%s ", prio);
	va_start(args, format);
	vfwprintf(stderr, format, args);
	va_end(args);
	fflush(stderr);
	
	ReleaseMutex(g_hLogMutex);

	return;
}



/*++

Print out an hexdump-like formatted representation of `data`.

--*/
void hexdump(PVOID data, SIZE_T size)
{
	WCHAR ascii[17] = { 0, };
	SIZE_T i, j;
	PBYTE ptr = (PBYTE)data;

	for (i = 0; i < size; ++i) 
	{
		BYTE c = ptr[i];

		if (!ascii[0])
			wprintf(L"%04Ix   ", i);

		wprintf(L"%02X ", c);

		ascii[i % 16] = (0x20 <= c && c < 0x7f) ? (WCHAR)c : L'.';

		if ((i + 1) % 8 == 0 || i + 1 == size)
		{
			wprintf(L" ");
			if ((i + 1) % 16 == 0)
			{
				wprintf(L"|  %s \n", ascii);
				ZeroMemory(ascii, sizeof(ascii));
			}
			else if (i + 1 == size)
			{
				ascii[(i + 1) % 16] = L'\0';
				if ((i + 1) % 16 <= 8)
				{
					wprintf(L" ");
				}
				for (j = (i + 1) % 16; j < 16; ++j)
				{
					wprintf(L"   ");
				}
				wprintf(L"|  %s \n", ascii);
			}
		}
	}

	return;
}


/*++

perror() style of function for Windows

--*/
void PrintError(const wchar_t* msg)
{
	WCHAR sysMsg[1024] = { 0, };

	DWORD eNum = GetLastError();
	FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, 
		eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		sysMsg, 
		(sizeof(sysMsg) - 1)/sizeof(WCHAR), 
		NULL
	);

	xlog(LOG_ERROR, L"%s, errcode=0x%x : %s", msg, eNum, sysMsg);
	return;
}


/*++

Routine Description:

Fill the buffer given in parameter with a (C-like) string filled with a random 
alpha-numeric charset.


Arguments:

	- str is a pointer to the buffer to fill with random chars

	- len is the number of random char to populate `str` with 


Return value:
	
	None

--*/
void GenerateRandomString(char* str, const size_t len) 
{
	static const char charset[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i) {
		str[i] = charset[rand() % (sizeof(charset) - 1)];
	}

	str[len] = 0;
}


/*++

Routine Description:

Create a (C-like) string filled with a random alpha-numeric charset.
The buffer must be free-ed (via LocalFree()) by the caller.


Arguments:

	- len is the length of the wanted random string


Return value:

	A pointer to the random string if successful, NULL otherwise.

--*/
char* CreateRandomString(const size_t len)
{
	char* m = LocalAlloc(LHND, len+1);
	if (!m)
		return NULL;

	GenerateRandomString(m, len);

	return m;
}


/*++

Routine Description:

Create a (C-like) wide string filled with a random alpha-numeric charset.
The buffer must be free-ed (via LocalFree()) by the caller.


Arguments:

	- len is the length of the wanted random string


Return value:

	A pointer to the random string if successful, NULL otherwise.

--*/
wchar_t* CreateRandomWideString(const size_t len)
{
	char* m = LocalAlloc(LHND, 2*(len + 1));
	if (!m)
		return NULL;

	GenerateRandomString(m, len);

	for (int i = 0; i < 2 * len; i+=2)
		m[i] = 0;

	return (wchar_t*)m;
}



/*++
 
--*/
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		break;
	
	case DLL_THREAD_ATTACH:
		break;
	
	case DLL_THREAD_DETACH:
		break;
	
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}
