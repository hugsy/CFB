#pragma once

#include "common.h"
#include <string>
#include <vector>
#include <Psapi.h>
#include <stdlib.h>

namespace Utils
{
	std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
	const byte* base64_decode(std::string const& encoded_string);
	std::vector<std::string> EnumerateDriversFromRoot();
};

