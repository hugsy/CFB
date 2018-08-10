#include <Windows.h>
#include <stdio.h>

#include "stdafx.h"

#include "../Common/common.h"
#include "client.h"
#include "utils.h"
#include "device.h"


static BOOLEAN g_DoRun = FALSE;


/**
 *
 */
VOID PrintHelpMenu()
{
	wprintf(
		L"?                     -- Print this help menu\n"
		L"quit                  -- Exit cleanly\n"
		L"hook <DriverName>     -- Add `DriverName' to the list of hooked drivers\n"
		L"unhook <DriverName>   -- Remove `DriverName' from the list of hooked drivers\n"
		L"count                 -- Returns the number of drivers hooked\n"
		// TODO: finish
	);
	return;
}


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

	BYTE lpBufferCommand[256];
	WCHAR lpBufferCommandW[sizeof(lpBufferCommand)];

	while (g_DoRun)
	{
		DWORD dwNumberOfBytesRead;
		LPWSTR* lpCommandEntries;
		DWORD dwNbEntries;

		RtlZeroMemory(lpBufferCommand, sizeof(lpBufferCommand));
		RtlZeroMemory(lpBufferCommandW, sizeof(lpBufferCommandW));

		wprintf(CLI_PROMPT);
		fflush(stdout);

		if (!ReadFile(hStdin, lpBufferCommand, sizeof(lpBufferCommand), &dwNumberOfBytesRead, NULL))
		{
			PrintError(L"ReadFile()");
			g_DoRun = FALSE;
			break;
		}

		size_t szNumConvertedChars = 0;
		mbstowcs_s(&szNumConvertedChars, lpBufferCommandW, sizeof(lpBufferCommandW), lpBufferCommand, _TRUNCATE);

		xlog(LOG_DEBUG, L"Received '%s'\n", lpBufferCommandW);

		StringWStrip((LPWSTR) lpBufferCommandW);

		lpCommandEntries = StringWSplit(lpBufferCommandW, L' ', &dwNbEntries);
		if (!lpCommandEntries)
		{
			xlog(LOG_ERROR, L"Failed to parse the command\n");
			continue;
		}

		xlog(LOG_DEBUG, L"Command '%s' has %d entries\n", lpCommandEntries[0], dwNbEntries);

#define ASSERT_ARGNUM(x) { \
								if (dwNbEntries != x) \
								{ \
									xlog(LOG_ERROR, L"Command '%s' expects 1 argument only\n", lpCommandEntries[0]); \
									break; \
								} \
						}

		do
		{
			if (!wcscmp(lpCommandEntries[0], L"quit"))
			{
				xlog(LOG_INFO, L"Exiting...\n");
				g_DoRun = FALSE;
				break;
			}

			if (!wcscmp(lpCommandEntries[0], L"?"))
			{
				PrintHelpMenu();
				break;
			}

			if (!wcscmp(lpCommandEntries[0], L"hook") || !wcscmp(lpCommandEntries[0], L"unhook"))
			{
				/*
				if (dwNbEntries != 2)
				{
					xlog(LOG_ERROR, L"Command '%1$s' expects 1 argument only\nExample: %1$s tcpip\n", lpCommandEntries[0]);
					break;
				}
				*/
				ASSERT_ARGNUM(2);

				LPWSTR lpDriver = lpCommandEntries[1];
				xlog(LOG_DEBUG, L"Trying to %s '%s'\n", lpCommandEntries[0], lpDriver);

				if (!wcscmp(lpCommandEntries[0], L"hook") && !HookDriver(lpDriver))
				{
					PrintError(L"HookDriver()");
				}
				else if (!wcscmp(lpCommandEntries[0], L"unhook") && !UnhookDriver(lpDriver))
				{
					PrintError(L"UnhookDriver()");
				}
				else
				{
					xlog(LOG_SUCCESS, L"Driver object '%s' is now %sed\n", lpDriver, lpCommandEntries[0]);
				}

				break;
			}

			if (!wcscmp(lpCommandEntries[0], L"count"))
			{
				DWORD dwNbDrivers;

				if (!GetNumberOfDrivers(&dwNbDrivers))
				{
					PrintError(L"GetNumberOfDrivers()");
				}
				else
				{
					xlog(LOG_SUCCESS, L"%d drivers hooked\n", dwNbDrivers);
				}

				break;
			}

			if (!wcscmp(lpCommandEntries[0], L"info"))
			{
				/*
				if (dwNbEntries != 2)
				{
					xlog(LOG_ERROR, L"Command '%1$s' expects 1 argument only\nExample: %1$s 1\n", lpCommandEntries[0]);
					break;
				}
				*/
				ASSERT_ARGNUM(2);

				HOOKED_DRIVER_INFO hDrvInfo;
				DWORD dwIndex = _wtoi(lpCommandEntries[1]);

				if (!GetHookedDriverInfo(dwIndex, &hDrvInfo))
				{
					PrintError(L"GetHookedDriverInfo()");
				}
				else
				{
					xlog(LOG_SUCCESS, L"Driver %d\n- Name '%s'\n-Enabled: %d\n",
						dwIndex, hDrvInfo.Name, hDrvInfo.Enabled);
				}
				break;
			}

			xlog(LOG_ERROR, L"Unknown command '%s'\n", lpCommandEntries[0]);

		} while (0);

		FreeAllSplittedElements(lpCommandEntries, dwNbEntries);
	}

	return;
}