#include "Connectors/Dummy.hpp"

#include <chrono>
#include <iostream>

#include "Log.hpp"


namespace CFB::Broker::Connectors
{

Dummy::Dummy()
{
    dbg("Initializing connector '%s'", Name().c_str());
}

Dummy::~Dummy()
{
    dbg("Terminating connector '%s'", Name().c_str());
}

std::string const
Dummy::Name() const
{
    return "Dummy";
}

Result<u32>
Dummy::IrpCallback(CFB::Comms::CapturedIrp const& Irp)
{
    SYSTEMTIME SystemTime {};
    if ( ::FileTimeToSystemTime((FILETIME*)&Irp.Header.TimeStamp, &SystemTime) )
    {
        std::ostringstream info, details;
        info << "New IRP: ";
        info << SystemTime.wYear << "/";
        info << SystemTime.wMonth << "/";
        info << SystemTime.wDay << " ";
        info << SystemTime.wHour << ":";
        info << SystemTime.wMinute << ":";
        info << SystemTime.wSecond << ":";
        info << SystemTime.wMilliseconds << " UTC";
        info("%s", info.str().c_str());
    }

    {
        std::ostringstream details;
        details << "Details:" << std::endl;
        details << "  - Driver: " << CFB::Utils::ToString(Irp.Header.DriverName) << std::endl;
        details << "  - Device: " << CFB::Utils::ToString(Irp.Header.DeviceName) << std::endl;
        details << "  - Process: " << CFB::Utils::ToString(Irp.Header.ProcessName) << " (PID:" << Irp.Header.Pid
                << ", TID:" << Irp.Header.Tid << ")" << std::endl;
        details << "  - IOCTL code: " << CFB::Utils::ToString(CFB::Comms::Ioctl {Irp.Header.IoctlCode}) << std::endl;
        details << std::hex;
        details << "  - Status: " << Irp.Header.Status << std::endl;
        details << "  - Major: " << CFB::Utils::IrpMajorToString((u32)Irp.Header.MajorFunction) << std::endl;
        details << "  - Minor: " << (u32)Irp.Header.MinorFunction << std::endl;
        details << "  - InLen: " << Irp.Header.InputBufferLength << std::endl;
        details << "  - OutLen: " << Irp.Header.OutputBufferLength << std::endl;
        dbg("%s", details.str().c_str());
    }

    if ( Irp.Header.InputBufferLength )
    {
        CFB::Utils::Hexdump((PVOID)Irp.InputBuffer.data(), Irp.InputBuffer.size(), "InputBuffer");
    }

    if ( Irp.Header.OutputBufferLength )
    {
        CFB::Utils::Hexdump((PVOID)Irp.OutputBuffer.data(), Irp.OutputBuffer.size(), "Output Buffer");
    }

    return Ok(0);
}

} // namespace CFB::Broker::Connectors
