#include "Connectors/Dummy.hpp"

#include <chrono>
#include <iostream>


namespace CFB::Broker::Connectors
{


std::string const
Dummy::Name() const
{
    return "Dummy";
}

Result<u32>
Dummy::IrpCallback(CFB::Comms::CapturedIrp const& Irp)
{
    std::cout << "New IRP: ";

    SYSTEMTIME SystemTime {};
    if ( ::FileTimeToSystemTime((FILETIME*)&Irp.Header.TimeStamp, &SystemTime) )
    {
        std::cout << SystemTime.wYear << "/";
        std::cout << SystemTime.wMonth << "/";
        std::cout << SystemTime.wDay << " ";
        std::cout << SystemTime.wHour << ":";
        std::cout << SystemTime.wMinute << ":";
        std::cout << SystemTime.wSecond << ":";
        std::cout << SystemTime.wMilliseconds << " UTC";
    }

    std::cout << std::endl << std::flush;
    std::wcout << L"  - Driver: " << Irp.Header.DriverName << std::endl;
    std::wcout << L"  - Device: " << Irp.Header.DeviceName << std::endl;
    std::cout << "  - IOCTL code: " << CFB::Utils::ToString(CFB::Comms::Ioctl {Irp.Header.IoctlCode}) << std::endl;
    std::cout << std::hex;
    std::cout << "  - Status: " << Irp.Header.Status << std::endl;
    std::cout << "  - Major: " << CFB::Utils::IrpMajorToString((u32)Irp.Header.MajorFunction) << std::endl;
    std::cout << "  - Minor: " << (u32)Irp.Header.MinorFunction << std::endl;
    std::cout << "  - InLen: " << Irp.Header.InputBufferLength << std::endl;
    std::cout << "  - OutLen: " << Irp.Header.OutputBufferLength << std::endl;

    std::cout << std::endl;

    return Ok(0);
}

} // namespace CFB::Broker::Connectors
