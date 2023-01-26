#pragma once

#include "Common.hpp"
#include "Utils.hpp"

#ifndef CFB_KERNEL_DRIVER
#include <codecvt>
#include <json.hpp>
#include <locale>
#include <sstream>
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

#ifndef CFB_KERNEL_DRIVER

struct CapturedIrp
{
    CapturedIrpHeader Header;
    std::vector<u8> InputBuffer;
    std::vector<u8> OutputBuffer;
};


static std::string
ToString(CapturedIrp const& Irp)
{
    std::ostringstream oss;
    oss << Irp.Header.TimeStamp << " " << CFB::Utils::ToString(Irp.Header.DriverName) << " "
        << CFB::Utils::ToString(Irp.Header.DeviceName) << " " << Irp.Header.Irql << Irp.Header.Type << " "
        << Irp.Header.MajorFunction << " " << Irp.Header.MinorFunction << " " << Irp.Header.IoctlCode << " "
        << Irp.Header.Pid << " " << Irp.Header.Tid << " " << CFB::Utils::ToString(Irp.Header.ProcessName) << " "
        << Irp.Header.Status << " " << Irp.Header.InputBufferLength << " " << Irp.Header.OutputBufferLength;
    return oss.str();
}

static std::string
ToJson(CapturedIrp const& Irp)
{
    std::ostringstream oss;
    oss << "{";
    oss << "\"TimeStamp\": " << Irp.Header.TimeStamp << ", ";
    oss << "\"DriverName\": \"" << CFB::Utils::ToString(Irp.Header.DriverName) << "\", ";
    oss << "\"DeviceName\": \"" << CFB::Utils::ToString(Irp.Header.DeviceName) << "\", ";
    oss << "\"Irql\": " << (u32)Irp.Header.Irql << ", ";
    oss << "\"Type\": " << (u32)Irp.Header.Type << ", ";
    oss << "\"MajorFunction\": " << (u32)Irp.Header.MajorFunction << ", ";
    oss << "\"MinorFunction\": " << (u32)Irp.Header.MinorFunction << ", ";
    oss << "\"IoctlCode\": " << Irp.Header.IoctlCode << ", ";
    oss << "\"Pid\": " << Irp.Header.Pid << ", ";
    oss << "\"Tid\": " << Irp.Header.Tid << ", ";
    oss << "\"ProcessName\": " << CFB::Utils::ToString(Irp.Header.ProcessName) << ", ";
    oss << "\"Status\": " << Irp.Header.Status << ", ";
    oss << "\"InputBufferLength\": " << Irp.Header.InputBufferLength << ", ";
    oss << "\"OutputBufferLength\": " << Irp.Header.OutputBufferLength;
    oss << "}";
    return oss.str();
}

static CapturedIrp
FromJson(nlohmann::json const& js)
{
    CapturedIrp Irp {};

    std::wstring DriverName  = CFB::Utils::ToWideString(js["DriverName"].get<std::string>());
    std::wstring DeviceName  = CFB::Utils::ToWideString(js["DeviceName"].get<std::string>());
    std::wstring ProcessName = CFB::Utils::ToWideString(js["ProcessName"].get<std::string>());

    RtlCopyMemory(Irp.Header.DriverName, DriverName.c_str(), DriverName.size() * sizeof(wchar_t));
    RtlCopyMemory(Irp.Header.DeviceName, DeviceName.c_str(), DeviceName.size() * sizeof(wchar_t));
    RtlCopyMemory(Irp.Header.ProcessName, ProcessName.c_str(), ProcessName.size() * sizeof(wchar_t));

    Irp.Header.TimeStamp          = js["TimeStamp"];
    Irp.Header.Irql               = js["Irql"];
    Irp.Header.Type               = js["Type"];
    Irp.Header.MajorFunction      = js["MajorFunction"];
    Irp.Header.MinorFunction      = js["MinorFunction"];
    Irp.Header.IoctlCode          = js["IoctlCode"];
    Irp.Header.Pid                = js["Pid"];
    Irp.Header.Tid                = js["Tid"];
    Irp.Header.Status             = js["Status"];
    Irp.Header.InputBufferLength  = js["InputBufferLength"];
    Irp.Header.OutputBufferLength = js["OutputBufferLength"];

    return Irp;
}
#endif // CFB_KERNEL_DRIVER

} // namespace CFB::Comms
