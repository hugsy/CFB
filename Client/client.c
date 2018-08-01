#include <Windows.h>
#include <stdio.h> 

#include "stdafx.h"

#include "../Common/common.h"
#include "client.h"


static BOOLEAN g_DoRun = FALSE;


/**
*
*/
static BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:
		xlog(LOG_INFO, L"Received Ctrl-C event, stopping...\n");
		g_DoRun = FALSE;
		return TRUE;

	case CTRL_CLOSE_EVENT:
		xlog(LOG_INFO, L"Received Ctrl-Close, stopping...\n");
		g_DoRun = FALSE;
		return TRUE;

	default:
		return FALSE;
	}
}


/**
*
*/
VOID RunInterpreter()
{
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	{
		xlog(LOG_ERROR, L"Failed to install CtrlHandler...\n");
	}

	HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

	if (hStdin == INVALID_HANDLE_VALUE)
	{
		xlog(LOG_CRITICAL, L"GetStdHandle failed: %lu\n", GetLastError());
		return;
	}


	xlog(LOG_INFO, L"Starting main loop...\n");
	g_DoRun = TRUE;


	while (g_DoRun)
	{
		BYTE lpBuffer[128];
		BYTE lpBufferW[2*sizeof(lpBuffer)+1];
		DWORD dwNumberOfBytesRead;

		RtlZeroMemory(lpBuffer, sizeof(lpBuffer));

		wprintf(CLI_PROMPT);
		fflush(stdout);

		if (!ReadFile(hStdin, lpBuffer, sizeof(lpBuffer), &dwNumberOfBytesRead, NULL))
		{
			xlog(LOG_CRITICAL, L"ReadFile() failed: %lu\n", GetLastError());
			g_DoRun = FALSE;
			break;
		}

		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, lpBufferW, sizeof(lpBufferW), lpBuffer, _TRUNCATE);


		xlog(LOG_INFO, L"Received command '%s'\n", lpBufferW);

		
	}

	return;
}