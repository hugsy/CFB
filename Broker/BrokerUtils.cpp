#include "BrokerUtils.hpp"

#include <wil/resource.h>

#include "Log.hpp"


namespace CFB::Broker::Utils
{

namespace Base64
{
static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";


static inline bool
is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}


std::string
Encode(std::vector<u8> const& bytes)
{
    return Encode(bytes.data(), bytes.size());
}


std::string
Encode(const u8* input_buffer, const usize input_buffer_len)
{
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    usize len = input_buffer_len;

    while ( len-- )
    {
        char_array_3[i++] = *(input_buffer++);
        if ( i == 3 )
        {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for ( i = 0; (i < 4); i++ )
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if ( i )
    {
        for ( j = i; j < 3; j++ )
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for ( j = 0; (j < i + 1); j++ )
            ret += base64_chars[char_array_4[j]];

        while ( (i++ < 3) )
            ret += '=';
    }

    return ret;
}


std::vector<u8>
Decode(std::string const& encoded_string)
{
    size_t in_len = encoded_string.size();
    int i         = 0;
    int j         = 0;
    int in_       = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<u8> ret;

    while ( in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]) )
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if ( i == 4 )
        {
            for ( i = 0; i < 4; i++ )
                char_array_4[i] = base64_chars.find(char_array_4[i]) & 0xff;

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for ( i = 0; (i < 3); i++ )
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if ( i )
    {
        for ( j = i; j < 4; j++ )
            char_array_4[j] = 0;

        for ( j = 0; j < 4; j++ )
            char_array_4[j] = base64_chars.find(char_array_4[j]) & 0xff;

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for ( j = 0; (j < i - 1); j++ )
            ret.push_back(char_array_3[j]);
    }

    return ret;
}
} // namespace Base64


Result<std::vector<std::pair<std::wstring, std::wstring>>>
EnumerateObjectDirectory(std::wstring const& Root)
{
    NTSTATUS Status = STATUS_SUCCESS;

    std::vector<std::pair<std::wstring, std::wstring>> ObjectList;

    wil::unique_handle hDirectory;
    ULONG ctx = 0;

    {
        HANDLE h;
        OBJECT_ATTRIBUTES oa;
        UNICODE_STRING name;

        ::RtlInitUnicodeString(&name, Root.c_str());
        InitializeObjectAttributes(&oa, &name, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

        Status = ::NtOpenDirectoryObject(&h, DIRECTORY_QUERY | DIRECTORY_TRAVERSE, &oa);
        if ( !NT_SUCCESS(Status) )
        {
            err("NtOpenDirectoryObject()");
            return Err(ErrorType::Code::InsufficientPrivilegeError);
        }

        hDirectory = wil::unique_handle(h);
    }

    do
    {
        ULONG rlen = 0;

        Status = ::NtQueryDirectoryObject(hDirectory.get(), nullptr, 0, true, false, &ctx, &rlen);
        if ( Status == STATUS_NO_MORE_ENTRIES )
        {
            break;
        }

        if ( Status != STATUS_BUFFER_TOO_SMALL )
        {
            CFB::Log::ntperror("NtQueryDirectoryObject()", Status);
            return Err(ErrorType::Code::InsufficientPrivilegeError);
        }

        auto buffer = std::make_unique<u8[]>(rlen);
        if ( !buffer )
        {
            err("allocation failed");
            return Err(ErrorType::Code::InsufficientPrivilegeError);
        }

        auto pObjDirInfo = reinterpret_cast<POBJECT_DIRECTORY_INFORMATION>(buffer.get());

        Status = ::NtQueryDirectoryObject(hDirectory.get(), pObjDirInfo, rlen, true, false, &ctx, &rlen);
        if ( NT_SUCCESS(Status) )
        {
            for ( ULONG i = 0; i < ctx; i++ )
            {
                if ( !pObjDirInfo[i].Name.Buffer || !pObjDirInfo[i].TypeName.Buffer )
                    break;

                ObjectList.push_back(std::make_pair(
                    std::wstring(pObjDirInfo[i].Name.Buffer),
                    std::wstring(pObjDirInfo[i].TypeName.Buffer)));
            }
        }
    } while ( true );

    return Ok(ObjectList);
}
} // namespace CFB::Broker::Utils
