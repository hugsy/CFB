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


static void
to_json(nlohmann::json& j, CapturedIrpHeader const& h)
{
    j["TimeStamp"]          = h.TimeStamp;
    j["DriverName"]         = CFB::Utils::ToString(h.DriverName);
    j["DeviceName"]         = CFB::Utils::ToString(h.DeviceName);
    j["Irql"]               = h.Irql;
    j["Type"]               = h.Type;
    j["MajorFunction"]      = h.MajorFunction;
    j["MinorFunction"]      = h.MinorFunction;
    j["IoctlCode"]          = h.IoctlCode;
    j["Pid"]                = h.Pid;
    j["Tid"]                = h.Tid;
    j["Status"]             = h.Status;
    j["ProcessName"]        = CFB::Utils::ToString(h.ProcessName);
    j["InputBufferLength"]  = h.InputBufferLength;
    j["OutputBufferLength"] = h.OutputBufferLength;
}

static void
from_json(const nlohmann::json& j, CapturedIrpHeader& h)
{
    j.at("TimeStamp").get_to<u64>(h.TimeStamp);
    j.at("Irql").get_to(h.Irql);
    j.at("Type").get_to(h.Type);
    j.at("MajorFunction").get_to(h.MajorFunction);
    j.at("MinorFunction").get_to(h.MinorFunction);
    j.at("IoctlCode").get_to(h.IoctlCode);
    j.at("Pid").get_to(h.Pid);
    j.at("Tid").get_to(h.Tid);
    j.at("Status").get_to(h.Status);
    j.at("InputBufferLength").get_to<u32>(h.InputBufferLength);
    j.at("OutputBufferLength").get_to<u32>(h.OutputBufferLength);

    auto const& ProcessName = CFB::Utils::ToWideString(j.at("ProcessName").get<std::string>());
    auto const& DriverName  = CFB::Utils::ToWideString(j.at("DriverName").get<std::string>());
    auto const& DeviceName  = CFB::Utils::ToWideString(j.at("DeviceName").get<std::string>());
    ::memcpy(
        h.ProcessName,
        ProcessName.c_str(),
        MIN(ProcessName.size() * sizeof(wchar_t), sizeof(h.ProcessName) - sizeof(wchar_t)));
    ::memcpy(
        h.DeviceName,
        DeviceName.c_str(),
        MIN(DeviceName.size() * sizeof(wchar_t), sizeof(h.DeviceName) - sizeof(wchar_t)));
    ::memcpy(
        h.DriverName,
        DriverName.c_str(),
        MIN(DriverName.size() * sizeof(wchar_t), sizeof(h.DriverName) - sizeof(wchar_t)));
}

static void
to_json(nlohmann::json& j, CapturedIrp const& i)
{
    j["Header"]       = i.Header;
    j["InputBuffer"]  = i.InputBuffer;
    j["OutputBuffer"] = i.OutputBuffer;
}

static void
from_json(const nlohmann::json& j, CapturedIrp& i)
{
    j.at("Header").get_to(i.Header);
    j.at("InputBuffer").get_to(i.InputBuffer);
    j.at("OutputBuffer").get_to(i.OutputBuffer);
}

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
#endif // CFB_KERNEL_DRIVER

} // namespace CFB::Comms
