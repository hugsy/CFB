#include <Windows.h>
#include <wchar.h>

#include "stdafx.h"

#include "../Common/common.h"


/**
* perror() style of function for Windows
*/
VOID PrintError(LPWSTR msg)
{
	DWORD eNum;
	WCHAR sysMsg[512] = { 0, };

	eNum = GetLastError();
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		sysMsg, sizeof(sysMsg) - 1, NULL);

	xlog(LOG_ERROR, L"%s, errcode=0x%x : %s", msg, eNum, sysMsg);
	return;
}


/**
 * Strips out the heading '\r' or '\n' and replace the trailing ones
 * with NULL bytes.
 */
VOID StringWStrip(LPWSTR wcStr)
{
	SIZE_T szStrLen = wcslen(wcStr)-1;
	LPWSTR lpStartPointer = wcStr;
	LPWSTR lpEndPointer = wcStr + szStrLen;

	/* null byte the trailing part */
	for (; *lpEndPointer == L'\n' || *lpEndPointer == L'\r'; lpEndPointer--)
		*lpEndPointer = L'\x00';

	/* shift heading part */
	for (; *lpStartPointer == L'\n' || *lpStartPointer == L'\r'; lpStartPointer++);
	wmemmove(wcStr, lpStartPointer, wcslen(lpStartPointer));

	return;
}


/**
* Return TRUE if `wcStr` starts with `wcPattern`.
*/
BOOL StringWStartsWith(LPWSTR wcStr, LPWSTR wcPattern)
{
	return wcsncmp(wcStr, wcPattern, wcslen(wcPattern)) == 0;
}


/**
 * Count the number of occurence of a WCHAR in a LPWSTR
 */
DWORD CountOccurence(LPWSTR wcStr, WCHAR p)
{
	DWORD dwNb = 0;
	for (LPWSTR ptr = wcStr; *ptr; ptr++)
	{
		if (*ptr == p)
		{
			dwNb++;
		}
	}
	return dwNb;
}


/**
 *
 */
LPWSTR* StringWSplit(LPWSTR wcStr, WCHAR p, LPDWORD lpdwNbEntries)
{
	DWORD dwNumberOfEntries = CountOccurence(wcStr, p);
	LPWSTR* lpEntries = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, (dwNumberOfEntries+1+1) * sizeof(LPWSTR));
	if (!lpEntries)
	{
		return NULL;
	}

	DWORD i = 0;
	LPWSTR lpStartPtr = wcStr;
	SIZE_T szOrigLen = wcslen(wcStr);

	for (DWORD j=0; j < szOrigLen+1; j++)
	{
		LPWSTR ptr = wcStr + j;
		if (*ptr == p || j == szOrigLen)
		{
			*ptr = L'\0';
			SIZE_T szCurArgLength = wcslen(lpStartPtr)+1;
			lpEntries[i] = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, szCurArgLength * sizeof(WCHAR));
			if (!lpEntries[i])
			{
				*ptr = p;
				break;
			}

			wcsncpy_s(lpEntries[i], szCurArgLength, lpStartPtr, _TRUNCATE);
			//wprintf(L"[%d] '%s' -> '%s' %llu\n", i, lpStartPtr, lpEntries[i], szCurArgLength);
			i++;
			*ptr = p;
			lpStartPtr += szCurArgLength;
		}
	}

	*lpdwNbEntries = i;
	return lpEntries;
}


/**
 *
 */
VOID FreeAllSplittedElements(LPWSTR* lpEntries, DWORD dwNbEntries)
{
	for (DWORD i = 0; i < dwNbEntries; i++)
	{
		LocalFree(lpEntries[i]);
	}

	LocalFree(lpEntries);

	return;
}