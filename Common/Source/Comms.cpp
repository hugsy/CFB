#pragma once

#include "Comms.hpp"

#include "Utils.hpp"

#ifndef CFB_KERNEL_DRIVER
// clang-format off
#include <codecvt>
#include <locale>
#include <sstream>

// clang-format on
#endif // CFB_KERNEL_DRIVER


namespace CFB::Comms
{
#ifdef CFB_KERNEL_DRIVER

#else
void
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

void
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

void
to_json(nlohmann::json& j, CapturedIrp const& i)
{
    j["Header"]       = i.Header;
    j["InputBuffer"]  = i.InputBuffer;
    j["OutputBuffer"] = i.OutputBuffer;
}

void
from_json(const nlohmann::json& j, CapturedIrp& i)
{
    j.at("Header").get_to(i.Header);
    j.at("InputBuffer").get_to(i.InputBuffer);
    j.at("OutputBuffer").get_to(i.OutputBuffer);

    assert(i.InputBuffer.size() == i.Header.InputBufferLength);
    assert(i.OutputBuffer.size() == i.Header.OutputBufferLength);
}
#endif // CFB_KERNEL_DRIVER

} // namespace CFB::Comms
