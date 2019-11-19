#include "Utils.h"


/**
 *
 * adapted from base64.cpp and base64.h by René Nyffenegger
 *
 */

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static inline bool is_base64(unsigned char c) 
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}


std::string Utils::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) 
{
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			ret += '=';

	}

	return ret;
}


std::vector<BYTE> Utils::base64_decode(std::string const& encoded_string)
{
	size_t in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::vector<BYTE> ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) 
	{
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i == 4) 
		{
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]) & 0xff;

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) 
	{
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]) & 0xff;

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) 
			ret.push_back(char_array_3[j]);
	}

	return ret;
}



std::vector<std::string> Utils::EnumerateDrivers()
{
	DWORD cbNeeded=0, dwSize=0, dwNbDrivers=0;

	BOOL bRes = ::EnumDeviceDrivers(NULL, 0, &cbNeeded);
	if (!bRes)
	{
		if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			throw std::runtime_error("Unexpected result from EnumDeviceDrivers1");
	}

	dwSize = cbNeeded;

	LPVOID* lpDrivers = (LPVOID*)::LocalAlloc(LPTR, dwSize);
	if(!lpDrivers)
		throw std::runtime_error("LocalAlloc");

	bRes = ::EnumDeviceDrivers(lpDrivers, dwSize, &cbNeeded);

	if (!bRes || dwSize != cbNeeded)
	{
		::LocalFree(lpDrivers);
		throw std::runtime_error("EnumDeviceDrivers2 failed");
	}


	std::vector<std::string> DriverList;
	CHAR wszDriverBaseName[MAX_PATH] = { 0, };
	dwNbDrivers = dwSize / sizeof(LPVOID);

	for (DWORD i = 0; i < dwNbDrivers; i++)
	{
		memset(wszDriverBaseName, 0, sizeof(wszDriverBaseName));

		if (::GetDeviceDriverBaseNameA(lpDrivers[i], wszDriverBaseName, _countof(wszDriverBaseName)))
			DriverList.push_back(std::string(wszDriverBaseName));
	}

	::LocalFree(lpDrivers);

	return DriverList;
}



std::vector<std::pair<std::wstring, std::wstring>> Utils::EnumerateObjectDirectory(std::wstring const& Root = L"\\")
{
	std::vector<std::pair<std::wstring, std::wstring>> ObjectList;

	HANDLE hDirectory;
	OBJECT_ATTRIBUTES ObjAttr;
	UNICODE_STRING usName;

	RtlInitUnicodeString(&usName, Root.c_str());
	InitializeObjectAttributes(&ObjAttr, &usName, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

	NTSTATUS Status = NtOpenDirectoryObject(&hDirectory, DIRECTORY_QUERY| DIRECTORY_TRAVERSE, &ObjAttr);
	if (!NT_SUCCESS(Status))
		RAISE_GENERIC_EXCEPTION("NtOpenDirectoryObject");


	ULONG ctx = 0;

	do 
	{
		ULONG rlen = 0;

		Status = NtQueryDirectoryObject(hDirectory, NULL, 0, TRUE, FALSE, &ctx, &rlen);
		if (Status != 0xC0000023) // BUFFER_TOO_SMALL
			break;

		POBJECT_DIRECTORY_INFORMATION pObjDirInfo = (POBJECT_DIRECTORY_INFORMATION)::LocalAlloc(LPTR, rlen);
		if (!pObjDirInfo)
			break;

		Status = NtQueryDirectoryObject(hDirectory, pObjDirInfo, rlen, TRUE, FALSE, &ctx, &rlen);
		if (NT_SUCCESS(Status))
		{
			for (ULONG i = 0; i < ctx; i++)
			{
				if (!pObjDirInfo[i].Name.Buffer || !pObjDirInfo[i].TypeName.Buffer)
					break;

				ObjectList.push_back(
					std::make_pair(
						std::wstring(pObjDirInfo[i].Name.Buffer),
						std::wstring(pObjDirInfo[i].TypeName.Buffer)
					)
				);
			}
		}
		else
		{
			::LocalFree(pObjDirInfo);
			break;
		}

		::LocalFree(pObjDirInfo);
	} 
	while (true);

	NtClose(hDirectory);


	return ObjectList;
}