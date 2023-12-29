#include "Connectors/Dummy.hpp"

#include <iostream>

#include "Log.hpp"
#include "Utils.hpp"

#define MAX_HEXDUMP_BYTES 256

#define xerr(fmt, ...)                                                                                                 \
    {                                                                                                                  \
        warn("[CFB::Broker::Manager::%s] " fmt, Name().c_str(), __VA_ARGS__);                                          \
    }

#define xwarn(fmt, ...)                                                                                                \
    {                                                                                                                  \
        warn("[CFB::Broker::Connector::%s] " fmt, Name().c_str(), __VA_ARGS__);                                        \
    }

#define xok(fmt, ...)                                                                                                  \
    {                                                                                                                  \
        ok("[CFB::Broker::Connector::%s] " fmt, Name().c_str(), __VA_ARGS__);                                          \
    }

#define xinfo(fmt, ...)                                                                                                \
    {                                                                                                                  \
        info("[CFB::Broker::Connector::%s] " fmt, Name().c_str(), __VA_ARGS__);                                        \
    }

#define xdbg(fmt, ...)                                                                                                 \
    {                                                                                                                  \
        dbg("[CFB::Broker::Connector::%s] " fmt, Name().c_str(), __VA_ARGS__);                                         \
    }


namespace CFB::Broker::Connectors
{

Dummy::Dummy()
{
    xdbg("Initializing connector '%s'", Name().c_str());
}

Dummy::~Dummy()
{
    xdbg("Terminating connector '%s'", Name().c_str());
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

    std::ostringstream details;
    details << "Details:" << std::endl;
    details << "  - Driver: " << CFB::Utils::ToString(Irp.Header.DriverName) << std::endl;
    details << "  - Device: " << CFB::Utils::ToString(Irp.Header.DeviceName) << std::endl;
    details << "  - Process: " << CFB::Utils::ToString(Irp.Header.ProcessName) << " (PID:" << Irp.Header.Pid
            << ", TID:" << Irp.Header.Tid << ")" << std::endl;
    if ( Irp.Header.MajorFunction == 0xe || Irp.Header.MajorFunction == 0xf )
    {
        details << "  - IOCTL code: " << CFB::Utils::ToString(CFB::Comms::Ioctl {Irp.Header.IoctlCode}) << std::endl;
    }
    details << std::hex;
    details << "  - Major: " << CFB::Utils::IrpMajorToString((u32)Irp.Header.MajorFunction) << std::endl;
    details << "  - Minor: " << (u32)Irp.Header.MinorFunction << std::endl;
    details << "  - InLen: " << Irp.Header.InputBufferLength << std::endl;
    details << "  - OutLen: " << Irp.Header.OutputBufferLength << std::endl;
    details << "  - Status: " << Irp.Header.Status << std::endl;
    dbg("%s", details.str().c_str());
    if ( Irp.Header.Status )
    {
        CFB::Log::ntperror("  - NTSTATUS", Irp.Header.Status);
    }

    if ( Irp.Header.InputBufferLength )
    {
        CFB::Utils::Hexdump(
            (PVOID)Irp.InputBuffer.data(),
            MIN(Irp.InputBuffer.size(), MAX_HEXDUMP_BYTES),
            "InputBuffer");
    }

    if ( Irp.Header.OutputBufferLength )
    {
        CFB::Utils::Hexdump(
            (PVOID)Irp.OutputBuffer.data(),
            MIN(Irp.OutputBuffer.size(), MAX_HEXDUMP_BYTES),
            "Output Buffer");
    }

    return Ok(0);
}

} // namespace CFB::Broker::Connectors
