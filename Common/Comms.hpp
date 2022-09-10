#include "Common.hpp"

namespace CFB::Comms
{

struct CapturedIrpHeader
{
    LARGE_INTEGER TimeStamp;
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
    usize InputBufferLength;
    usize OutputBufferLength;
};

} // namespace CFB::Comms
