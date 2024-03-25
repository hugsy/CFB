#include "Utils.hpp"

#ifdef CFB_KERNEL_DRIVER
#else
#include <algorithm>
#include <iostream>
#include <sstream>
#endif // CFB_KERNEL_DRIVER

namespace CFB::Utils
{
void
Hexdump(const PVOID data, SIZE_T size, PCSTR header, SIZE_T base)
{
    CHAR ascii[17] {};
    auto ptr = reinterpret_cast<u8*>(data);

    if ( header )
    {
        XPRINTF("%s\n", header);
    }

    for ( size_t i = 0; i < size; ++i )
    {
        u8 c = ptr[i];

        if ( !ascii[0] )
            XPRINTF("%08Ix   ", base + i);

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

const char*
IrpMajorToString(u32 type)
{
    switch ( type )
    {
    case 0x00:
        return "IRP_MJ_CREATE";
    case 0x01:
        return "IRP_MJ_CREATE_NAMED_PIPE";
    case 0x02:
        return "IRP_MJ_CLOSE";
    case 0x03:
        return "IRP_MJ_READ";
    case 0x04:
        return "IRP_MJ_WRITE";
    case 0x05:
        return "IRP_MJ_QUERY_INFORMATION";
    case 0x06:
        return "IRP_MJ_SET_INFORMATION";
    case 0x07:
        return "IRP_MJ_QUERY_EA";
    case 0x08:
        return "IRP_MJ_SET_EA";
    case 0x09:
        return "IRP_MJ_FLUSH_BUFFERS";
    case 0x0a:
        return "IRP_MJ_QUERY_VOLUME_INFORMATION";
    case 0x0b:
        return "IRP_MJ_SET_VOLUME_INFORMATION";
    case 0x0c:
        return "IRP_MJ_DIRECTORY_CONTROL";
    case 0x0d:
        return "IRP_MJ_FILE_SYSTEM_CONTROL";
    case 0x0e:
        return "IRP_MJ_DEVICE_CONTROL";
    case 0x0f:
        return "IRP_MJ_INTERNAL_DEVICE_CONTROL";
    case 0x10:
        return "IRP_MJ_SHUTDOWN";
    case 0x11:
        return "IRP_MJ_LOCK_CONTROL";
    case 0x12:
        return "IRP_MJ_CLEANUP";
    case 0x13:
        return "IRP_MJ_CREATE_MAILSLOT";
    case 0x14:
        return "IRP_MJ_QUERY_SECURITY";
    case 0x15:
        return "IRP_MJ_SET_SECURITY";
    case 0x16:
        return "IRP_MJ_POWER";
    case 0x17:
        return "IRP_MJ_SYSTEM_CONTROL";
    case 0x18:
        return "IRP_MJ_DEVICE_CHANGE";
    case 0x19:
        return "IRP_MJ_QUERY_QUOTA";
    case 0x1a:
        return "IRP_MJ_SET_QUOTA";
    case 0x1b:
        return "IRP_MJ_PNP_POWER";
    }
    return "UnknownIrpType";
}

const char*
IrpTypeToString(u8 Type)
{
    switch ( Type )
    {
    case 0:
        return "IRP";
    case 1:
        return "FastIRP";
    }

    return "Unknown";
}

const char*
IrqlToString(u32 type)
{
    switch ( type )
    {
    case 0:
        return "PASSIVE_LEVEL";
    case 1:
        return "APC_LEVEL";
    case 2:
        return "DISPATCH_LEVEL";
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
        return "DEVICE_IRQL";
    case 13:
        return "CLOCK_LEVEL";
    case 14:
        return "POWER_LEVEL";
    case 15:
        return "HIGH_LEVEL";
    }
    return "Unknown";
}

#ifdef CFB_KERNEL_DRIVER
#else
std::string
ToString(std::wstring const& wide_string)
{
    int size_needed =
        ::WideCharToMultiByte(CP_UTF8, 0, wide_string.data(), (int)wide_string.size(), nullptr, 0, nullptr, nullptr);

    std::string str(size_needed, 0);

    if ( 0 == ::WideCharToMultiByte(
                  CP_UTF8,
                  0,
                  wide_string.data(),
                  (int)wide_string.size(),
                  str.data(),
                  (int)str.size(),
                  nullptr,
                  nullptr) )
    {
        str.clear();
    }
    return str;
}

std::wstring
ToWideString(std::string const& str)
{
    int size_needed = ::MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);

    std::wstring wstr(size_needed, 0);

    if ( 0 == ::MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), wstr.data(), (int)wstr.size()) )
    {
        wstr.clear();
    }
    return wstr;
}

std::string
ToString(CFB::Comms::Ioctl IoctlCode)
{
    u32 code           = (u32)IoctlCode;
    u32 DeviceType     = ((code >> 16) & 0x0000ffff);
    u32 AccessType     = ((code >> 14) & 0x00000003);
    u32 MethodType     = (code & 0x0000003);
    u32 FunctionNumber = ((code >> 2) & 0x0000fff);

    std::ostringstream oss;
    oss << "CTL_CODE(DeviceType=0x" << std::hex << DeviceType << ", Function=0x" << FunctionNumber;
    oss << ", Method=" << MethodType << ", Access=" << AccessType << ")";
    return oss.str();
}

std::string
ToString(CFB::Comms::RequestId id)
{
    using CFB::Comms::RequestId;
#define CaseToString(x)                                                                                                \
    {                                                                                                                  \
    case (x):                                                                                                          \
        return #x;                                                                                                     \
    }

    switch ( id )
    {
        CaseToString(RequestId::InvalidId);
        CaseToString(RequestId::HookDriver);
        CaseToString(RequestId::UnhookDriver);
        CaseToString(RequestId::GetNumberOfDrivers);
        CaseToString(RequestId::GetNamesOfDrivers);
        CaseToString(RequestId::GetDriverInfo);
        CaseToString(RequestId::SetEventPointer);
        CaseToString(RequestId::EnableMonitoring);
        CaseToString(RequestId::DisableMonitoring);
        CaseToString(RequestId::GetPendingIrp);
        CaseToString(RequestId::EnumerateDriverObject);
        CaseToString(RequestId::EnumerateDeviceObject);
    }
#undef CaseToString

    return "RequestId::Unknown";
}

std::string
ToString(CFB::Comms::CapturedIrp const& Irp)
{
    std::ostringstream oss;
    oss << Irp.Header.TimeStamp << " " << CFB::Utils::ToString(Irp.Header.DriverName) << " "
        << CFB::Utils::ToString(Irp.Header.DeviceName) << " " << Irp.Header.Irql << Irp.Header.Type << " "
        << Irp.Header.MajorFunction << " " << Irp.Header.MinorFunction << " "
        << CFB::Utils::IrpMajorToString(Irp.Header.MajorFunction) << " " << Irp.Header.IoctlCode << " "
        << Irp.Header.Pid << " " << Irp.Header.Tid << " " << CFB::Utils::ToString(Irp.Header.ProcessName) << " "
        << Irp.Header.Status << " " << Irp.Header.InputBufferLength << " " << Irp.Header.OutputBufferLength;
    return oss.str();
}
#endif // CFB_KERNEL_DRIVER

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
