#pragma once

#include "common.h"
#include <string>


class Utils
{
public:
	static std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
	static std::string base64_decode(std::string const& encoded_string);
};

