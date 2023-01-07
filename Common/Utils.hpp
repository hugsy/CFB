#pragma once

#include "Common.hpp"

#ifdef CFB_KERNEL_DRIVER
#define XPRINTF(...)                                                                                                   \
    {                                                                                                                  \
        ::DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __VA_ARGS__);                                             \
    }

#else
#include <stdio.h>

#define XPRINTF(...)                                                                                                   \
    {                                                                                                                  \
        printf(__VA_ARGS__);                                                                                           \
    }
#endif // CFB_KERNEL_DRIVER

#define WIDEN2(x) L##x
#define WIDEN(x) WIDEN2(x)

namespace CFB::Utils
{
///
/// @brief Print out an hexdump-like version of the buffer
///
/// @param data
/// @param size
///
void
Hexdump(PVOID data, SIZE_T size);

namespace Memory
{
///
/// @brief
///
/// @param Value
/// @param Base
/// @return true
/// @return false
///
bool
IsAligned(uptr const Value, usize const Base);

///
///@brief Align a value to a base
///
///@param Value
///@param Base
///@return uptr
///
uptr
AlignValue(uptr const Value, usize const Base);
} // namespace Memory
} // namespace CFB::Utils
