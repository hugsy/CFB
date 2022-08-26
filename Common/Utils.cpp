#include "Utils.hpp"

namespace CFB::Utils
{
void
Hexdump(PVOID data, SIZE_T size) // TODO: ugly, improve
{
    CHAR ascii[17] = {
        0,
    };
    auto ptr = reinterpret_cast<u8*>(data);

    for ( size_t i = 0; i < size; ++i )
    {
        u8 c = ptr[i];

        if ( !ascii[0] )
            XPRINTF("%04Ix   ", i);

        XPRINTF("%02X ", c);

        ascii[i % 16] = (0x20 <= c && c < 0x7f) ? (u8)c : '.';

        if ( (i + 1) % 8 == 0 || i + 1 == size )
        {
            XPRINTF(" ");
            if ( (i + 1) % 16 == 0 )
            {
                XPRINTF("|  %s \n", ascii);
                ::RtlSecureZeroMemory(ascii, sizeof(ascii));
            }
            else if ( i + 1 == size )
            {
                ascii[(i + 1) % 16] = '\0';
                if ( (i + 1) % 16 <= 8 )
                {
                    XPRINTF(" ");
                }
                for ( auto j = (i + 1) % 16; j < 16; ++j )
                {
                    XPRINTF("   ");
                }

                XPRINTF("|  %s \n", ascii);
            }
        }
    }

    return;
}

namespace Memory
{
bool
IsAligned(uptr const Value, usize const Base)
{
    return (Value & (Base - 1)) == 0;
}

uptr
AlignValue(uptr const Value, usize const Base)
{
    if ( IsAligned(Value, Base) )
    {
        return Value;
    }

    return (Value & ~(Base - 1)) + Base;
}
} // namespace Memory

} // namespace CFB::Utils
