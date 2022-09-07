#pragma once

#include "common.h"
#include "nt.h"

#include <string>
#include <vector>
#include <Psapi.h>
#include <stdlib.h>


#define MAX_REGSZ_SIZE 255
#define MAX_USERNAME_SIZE 255


namespace Utils
{
	std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
	std::vector<BYTE> base64_decode(std::string const& encoded_string);
	std::string WideStringToString(const std::wstring& original);

	std::vector<std::string> EnumerateDrivers();
	std::vector<std::pair<std::wstring, std::wstring>> EnumerateObjectDirectory(const std::wstring& Root);

	namespace Io
	{
		DWORD DeviceIoControlWrapper(
			const char* lpszDeviceName,
			const DWORD dwIoctlCode,
			const PBYTE lpInputBuffer,
			const DWORD dwInputBufferLength,
			PBYTE lpOutputBuffer,
			PDWORD lpdwOutputBufferLength
		);
	}

	namespace Registry
	{

		DWORD ReadDword(
			HKEY hKeyRoot,
			const std::wstring& SubKey,
			const std::wstring& KeyName,
			PDWORD lpdwKeyValue
		);

		BOOL ReadBool(
			HKEY hKeyRoot,
			const std::wstring& SubKey,
			const std::wstring& KeyName,
			PBOOL lpbKeyValue
		);

		DWORD ReadWString(
			HKEY hKeyRoot,
			const std::wstring& SubKey,
			const std::wstring& KeyName,
			std::wstring& KeyValue
		);
	};

	namespace Process
	{
		DWORD GetIntegrityLevel(std::wstring& IntegrityLevelName);
	}
};

