#include <Windows.h>
#include <stdio.h>

#include "stdafx.h"

#include "common.h"



HANDLE g_ConsoleMutex;


static void init()
{
	g_ConsoleMutex = CreateMutex(NULL, FALSE, NULL);
	return;
}


static void fini()
{
	CloseHandle(g_ConsoleMutex);
	return;
}


void _xlog(log_level_t level, const wchar_t* format, ...)
{
	size_t fmt_len;
	LPWSTR fmt;
	DWORD dwWaitResult;
	va_list args;
	const wchar_t* prio;

#ifndef _DEBUG
	/* If we're not in Debug mode, we don't care about xlog(LOG_DEBUG) */
	if (level == LOG_DEBUG)
		return;
#endif

	switch (level)
	{
	case LOG_DEBUG:
		prio = L"[DEBUG] ";
		break;
	case LOG_INFO:
		prio = L"[INFO] ";
		break;
	case LOG_SUCCESS:
		prio = L"[SUCCESS] ";
		break;
	case LOG_WARNING:
		prio = L"[WARNING] ";
		break;
	case LOG_ERROR:
		prio = L"[ERROR] ";
		break;
	case LOG_CRITICAL:
		prio = L"[CRITICAL] ";
		break;
	default:
		return;
	}

	fmt_len = wcslen(format) + wcslen(prio) + 4;
	fmt = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, fmt_len*sizeof(wchar_t));

	va_start(args, format);
	swprintf(fmt, fmt_len, L"%s %s", prio, format);

	SYSTEMTIME lt;
	GetLocalTime(&lt);

	dwWaitResult = WaitForSingleObject(g_ConsoleMutex, INFINITE);

	if (dwWaitResult == WAIT_OBJECT_0)
	{
#ifdef _DEBUG
		fwprintf(stderr, L"%02d-%02d-%02d %02d:%02d:%02d ",
			lt.wYear, lt.wMonth, lt.wDay,
			lt.wHour, lt.wMinute, lt.wSecond);
#endif
		vfwprintf(stderr, fmt, args);
		fflush(stderr);
	}
	va_end(args);
	ReleaseMutex(g_ConsoleMutex);

	LocalFree(fmt);

	return;
}


/**
 *
 */
static inline void __hexdump_thread_safe(PVOID data, SIZE_T size)
{
	WCHAR ascii[17] = { 0, };
	SIZE_T i, j;

	for (i = 0; i < size; ++i) {
		BYTE c = *((PCHAR)data + i);

		if (!ascii[0])
			wprintf(L"%llx   ", i);

		wprintf(L"%02X ", c);

		if (0x20 <= c && c < 0x7f)
		{
			ascii[i % 16] = (WCHAR)c;
		}
		else {
			ascii[i % 16] = L'.';
		}

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
}


/**
 *
 */
void Hexdump(PVOID data, SIZE_T size)
{
	DWORD dwWaitResult = WaitForSingleObject(g_ConsoleMutex, INFINITE);

	if (dwWaitResult == WAIT_OBJECT_0)
	{
		__hexdump_thread_safe(data, size);
	}

	ReleaseMutex(g_ConsoleMutex);
}



/**
 *
 */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		init();
		break;
	/*
	case DLL_THREAD_ATTACH:
		break;
	*/
	case DLL_THREAD_DETACH:
		fini();
		break;

	/*
	case DLL_PROCESS_DETACH:
		break;
	*/
	}


	return TRUE;
}