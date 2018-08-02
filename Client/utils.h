#pragma once

VOID StringWStrip(LPWSTR wcStr);
BOOLEAN StringWStartsWith(LPWSTR wcStr, LPWSTR wcPattern);
LPWSTR* StringWSplit(LPWSTR wcStr, WCHAR p, LPDWORD lpdwNbEntries);
VOID FreeAllSplittedElements(LPWSTR* lpEntries, DWORD dwNbEntries);
DWORD CountOccurence(LPWSTR wcStr, WCHAR p);