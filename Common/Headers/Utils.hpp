#pragma once

#include "Common.hpp"
#include "Comms.hpp"

#ifdef CFB_KERNEL_DRIVER
#define XPRINTF(...)                                                                                                   \
    {                                                                                                                  \
        ::DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, __VA_ARGS__);                                             \
    }

#else

#include <stdio.h>

#include <string>

#include "IoctlCodes.hpp"

#ifdef CFB_KERNEL_DRIVER
#else
#include "Messages.hpp"
#endif // CFB_KERNEL_DRIVER

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
/// @brief Print out an hexdump-like version of the buffer.
///
/// @param data A pointer to the buffer to hexdump
/// @param size The size of the buffer
/// @param header If not null, an header to print before the hexdump
/// @param base Specifies the base address
///
void
Hexdump(PVOID data, SIZE_T size, PCSTR header = nullptr, SIZE_T base = 0);


#ifdef CFB_KERNEL_DRIVER
#else
std::string
ToString(std::wstring const& input);

std::wstring
ToWideString(std::string const& input);

std::string
ToString(CFB::Comms::Ioctl code);

std::string
ToString(CFB::Comms::RequestId id);

std::string
ToString(CFB::Comms::CapturedIrp const& Irp);

std::string
IrpMajorToString(u32 type);
#endif // CFB_KERNEL_DRIVER

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
