#pragma once

#include "common.h"
#include "nt.h"

#include <string>
#include <vector>
#include <Psapi.h>
#include <stdlib.h>


namespace Utils
{
	std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
	std::vector<BYTE> base64_decode(std::string const& encoded_string);
	std::vector<std::string> EnumerateDrivers();
	std::vector<std::pair<std::wstring, std::wstring>> EnumerateObjectDirectory(const std::wstring& Root);
	std::string WideStringToString(const std::wstring& original);

	DWORD DeviceIoControlWrapper(
		const char* lpszDeviceName,
		const DWORD dwIoctlCode,
		const PBYTE lpInputBuffer,
		const DWORD dwInputBufferLength,
		PBYTE lpOutputBuffer,
		const DWORD dwOutputBufferLength
	);
};

