#pragma once

#include "Common.hpp"

#ifdef CFB_KERNEL_DRIVER
#define XPRINTF(...) {KdPrint((__VA_ARGS__));}

#else
#include <stdio.h>

#define XPRINTF(...) {printf(__VA_ARGS__);}
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
    void Hexdump(PVOID data, SIZE_T size);

    ///
    /// @brief Create a Random String object
    ///
    /// @param len
    /// @return std::string
    ///
    // std::string CreateRandomString(const usize len);

} // namespace CFB::Utils
