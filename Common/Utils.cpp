#include "Utils.hpp"

namespace CFB::Utils
{
    void Hexdump(PVOID data, SIZE_T size) // TODO: ugly, improve
    {
        CHAR ascii[17] = {
            0,
        };
        auto ptr = reinterpret_cast<u8*>(data);

        for (size_t i = 0; i < size; ++i)
        {
            u8 c = ptr[i];

            if (!ascii[0])
                XPRINTF("%04Ix   ", i);

            XPRINTF("%02X ", c);

            ascii[i % 16] = (0x20 <= c && c < 0x7f) ? (u8)c : '.';

            if ((i + 1) % 8 == 0 || i + 1 == size)
            {
                XPRINTF(" ");
                if ((i + 1) % 16 == 0)
                {
                    XPRINTF("|  %s \n", ascii);
                    ::RtlSecureZeroMemory(ascii, sizeof(ascii));
                }
                else if (i + 1 == size)
                {
                    ascii[(i + 1) % 16] = '\0';
                    if ((i + 1) % 16 <= 8)
                    {
                        XPRINTF(" ");
                    }
                    for (auto j = (i + 1) % 16; j < 16; ++j)
                    {
                        XPRINTF("   ");
                    }

                    XPRINTF("|  %s \n", ascii);
                }
            }
        }

        return;
    }

    // static void GenerateRandomString(char *str, const usize len)
    // {
    //     static const char charset[] =
    //         "0123456789"
    //         "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    //         "abcdefghijklmnopqrstuvwxyz";

    //     for (int i = 0; i < len - 1; ++i)
    //     {
    //         str[i] = charset[rand() % (sizeof(charset) - 1)];
    //     }

    //     str[len - 1] = 0;
    // }

    // std::string CreateRandomString(const usize len)
    // {
    //     std::string out;
    //     out.reserve(len+1);
    //     GenerateRandomString(out.c_str(), len);
    //     return out;
    // }


    // wchar_t *CreateRandomWideString(const size_t len)
    // {
    //     char *m = LocalAlloc(LHND, 2 * (len + 1));
    //     if (!m)
    //         return NULL;

    //     GenerateRandomString(m, len);

    //     for (int i = 0; i < 2 * len; i += 2)
    //         m[i] = 0;

    //     return (wchar_t *)m;
    // }

} // namespace CFB::Utils
