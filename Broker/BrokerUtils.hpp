#pragma once

// clang-format off
#include "Broker.hpp"


// clang-format on


namespace CFB::Broker::Utils
{

namespace Base64
{

///
/// @brief Encode a vector of bytes to base64
///
/// @param bytes
/// @return std::string
///
std::string
Encode(std::vector<u8> const& bytes);

///
/// @brief Encode a char* to base64
///
/// @param bytes_to_encode
/// @param in_len
/// @return std::string
///
std::string
Encode(const u8* bytes_to_encode, const usize in_len);

///
/// @brief Decode a given base64-encoded string to an array of bytes
///
/// @param encoded_string
/// @return std::vector<u8>
///
std::vector<u8>
Decode(std::string const& encoded_string);
} // namespace Base64


///
/// @brief Helper function to enumerate objects from the Object Manager
///
/// @param Root the root to start dumping objects
/// @return Result<std::vector<std::pair<std::wstring, std::wstring>>>
///
Result<std::vector<std::pair<std::wstring, std::wstring>>>
EnumerateObjectDirectory(std::wstring const& Root = L"\\");


///
/// @brief Try to acquire a privilege from its name
///
/// @param lpszPrivilegeName
/// @return Result<bool>
///
Result<bool>
AcquirePrivileges(std::vector<std::wstring_view> const& privilege_names);


///
/// @brief
///
/// @param lpszPrivilegeName
/// @param lpHasPriv
/// @return Result<bool>
///
Result<bool>
HasPrivilege(std::wstring_view const& privilege_name);


} // namespace CFB::Broker::Utils
