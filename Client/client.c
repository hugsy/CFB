#include <Windows.h>
#include <stdio.h>
#include <psapi.h>

#include "stdafx.h"

#include "../Common/common.h"
#include "client.h"
#include "utils.h"
#include "device.h"


static BOOLEAN g_DoRun = FALSE;


/*++

Print the help menu

--*/
VOID PrintHelpMenu()
{
	wprintf(
		L"?                     -- Print this help menu\n"
		L"quit                  -- Exit cleanly\n"
		L"hook <DrvName>        -- Add `DrvName' to the list of hooked drivers (ex. '\\driver\\HEVD')\n"
		L"unhook <DrvName>      -- Remove `DrvName' from the list of hooked drivers (ex. '\\driver\\HEVD')\n"
		L"count                 -- Returns the number of drivers hooked\n"
		L"info <DrvIdx>         -- Displays some info about the hooked driver number DrvIdx (ex. 2)\n"
		L"enum                  -- Enumerate all drivers on the system\n"
	);
	return;
}


/*++

Adjusted version from MSDN
https://docs.microsoft.com/en-us/windows/desktop/psapi/enumerating-all-device-drivers-in-the-system

--*/
BOOL EnumerateDrivers()
{
	LPVOID lpDrivers[2048];
	DWORD cbNeeded;
	int cDrivers;

	if (EnumDeviceDrivers(lpDrivers, sizeof(lpDrivers), &cbNeeded) && cbNeeded < sizeof(lpDrivers))
	{
		WCHAR wszDriver[2048] = { 0, };

		cDrivers = cbNeeded / sizeof(LPVOID);

		for (int i = 0; i < cDrivers; i++)
		{
			if (GetDeviceDriverBaseName(lpDrivers[i], wszDriver, sizeof(wszDriver) / sizeof(WCHAR)))
			//if (GetDeviceDriverFileName(lpDrivers[i], wszDriver, sizeof(wszDriver) / sizeof(WCHAR)))
			{
				xlog(LOG_SUCCESS, L"%d: %s\n", i + 1, wszDriver);
			}
			else
			{
				PrintError(L"GetDeviceDriverBaseName()");
				return FALSE;
			}
		}
	}
	else
	{
		xlog(LOG_ERROR, L"EnumDeviceDrivers() failed; array size needed is %d\n", cbNeeded / sizeof(LPVOID));
		return FALSE;
	}

	return TRUE;
}


/*++

--*/
static BOOL CtrlHandler(DWORD fdwCtrlType)
{
	BOOL bRes = TRUE;

	xlog(LOG_DEBUG, L"In CtrlHandler)()\n");

	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:
		xlog(LOG_INFO, L"Received Ctrl-C event, stopping...\n");
		g_DoRun = FALSE;
		break;

	case CTRL_CLOSE_EVENT:
		xlog(LOG_INFO, L"Received Ctrl-Close, stopping...\n");
		g_DoRun = FALSE;
		break;

	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		xlog(LOG_INFO, L"Received stop from system...\n");
		g_DoRun = FALSE;
		break;

	default:
		xlog(LOG_ERROR, L"Unhandled control signal\n");
		bRes = FALSE;
		break;
	}

	return bRes;
}



/*++

--*/
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


	xlog(LOG_INFO, L"Everything is ready!! Starting main loop... (Press Ctrl-C or type 'quit' to exit)\n");

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
		}

		if (!g_DoRun)
			break;

		size_t szNumConvertedChars = 0;
		mbstowcs_s(&szNumConvertedChars,
			       lpBufferCommandW,
			       sizeof(lpBufferCommandW)/sizeof(WCHAR),
			       lpBufferCommand,
			       dwNumberOfBytesRead);

		StringWStrip((LPWSTR) lpBufferCommandW);

		lpCommandEntries = StringWSplit(lpBufferCommandW, L' ', &dwNbEntries);
		if (!lpCommandEntries || !dwNbEntries)
		{
			xlog(LOG_ERROR, L"Failed to parse the command\n");
			continue;
		}

		DWORD dwNbArgs = dwNbEntries - 1;

#ifdef _DEBUG
		xlog(LOG_DEBUG, L"Received command '%s' with %d arguments\n", lpCommandEntries[0], dwNbArgs);

		for(DWORD i=0; i<dwNbArgs; i++)
			xlog(LOG_DEBUG, L"Arg[%d] = '%s'\n", i, lpCommandEntries[1+i]);

#endif


#define HANDLE_COMMAND(x) if (!wcscmp(lpCommandEntries[0], x))

#define ASSERT_ARGNUM(x) { \
								if ( dwNbArgs < x ) \
								{ \
									xlog(LOG_ERROR, L"Command '%s' expects %d argument(s)\n", lpCommandEntries[0], x); \
									break; \
								} \
						}

		do
		{

			HANDLE_COMMAND(L"quit")
			{
				xlog(LOG_INFO, L"Exiting...\n");
				g_DoRun = FALSE;
				break;
			}


			HANDLE_COMMAND(L"?")
			{
				PrintHelpMenu();
				break;
			}

			HANDLE_COMMAND(L"enum")
			{
				EnumerateDrivers();
				break;
			}

			HANDLE_COMMAND(L"hook")
			{
				ASSERT_ARGNUM(1);

				xlog(LOG_DEBUG, L"Hook command to '%s' (%dB)\n", lpCommandEntries[1], wcslen(lpCommandEntries[1]));

				if( !HookDriver(lpCommandEntries[1]) )
				{
					PrintError(L"HookDriver()");
				}
				else
				{
					xlog(LOG_SUCCESS, L"Driver '%s' is now hooked\n", lpCommandEntries[1]);
				}

				break;
			}


			HANDLE_COMMAND(L"unhook")
			{
				ASSERT_ARGNUM(1);

				xlog(LOG_DEBUG, L"Unhook command to '%s'\n", lpCommandEntries[1]);

				if ( !UnhookDriver(lpCommandEntries[1]) )
				{
					PrintError(L"UnhookDriver()");
				}
				else
				{
					xlog(LOG_SUCCESS, L"Driver '%s' is now unhooked\n", lpCommandEntries[1]);
				}

				break;
			}


			HANDLE_COMMAND(L"count")
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


			HANDLE_COMMAND(L"info")
			{
				ASSERT_ARGNUM(1);

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

	xlog(LOG_DEBUG, L"Ending thread of RunInterpreter()...\n");

	return;
}