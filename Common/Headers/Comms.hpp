#pragma once

#include "Common.hpp"

#ifndef CFB_KERNEL_DRIVER
// clang-format off
#include <nlohmann/json.hpp>
// clang-format on
#endif // CFB_KERNEL_DRIVER


namespace CFB::Comms
{
#pragma pack(push, 1)
struct CapturedIrpHeader
{
    u64 TimeStamp;
    wchar_t DriverName[CFB_DRIVER_MAX_PATH];
    wchar_t DeviceName[CFB_DRIVER_MAX_PATH];
    u8 Irql;
    u8 Type;
    u8 MajorFunction;
    u8 MinorFunction;
    u32 IoctlCode;
    u32 Pid;
    u32 Tid;
    NTSTATUS Status;
    wchar_t ProcessName[CFB_DRIVER_MAX_PATH];
    u32 InputBufferLength;
    u32 OutputBufferLength;
};
#pragma pack(pop)

#ifdef CFB_KERNEL_DRIVER
#else
struct CapturedIrp
{
    CapturedIrpHeader Header;
    std::vector<u8> InputBuffer;
    std::vector<u8> OutputBuffer;
};

void
to_json(nlohmann::json& j, CapturedIrpHeader const& h);

void
from_json(const nlohmann::json& j, CapturedIrpHeader& h);

void
to_json(nlohmann::json& j, CapturedIrp const& i);

void
from_json(const nlohmann::json& j, CapturedIrp& i);
#endif // CFB_KERNEL_DRIVER

} // namespace CFB::Comms
