#include "Messages.hpp"

#ifndef CFB_KERNEL_DRIVER
#include <codecvt>
#include <locale>

#include "Utils.hpp"


namespace CFB::Comms
{
//
// JSON Converters
//
void
to_json(json& dst, const DriverRequest& src)
{
    dst["id"] = src.Id;
    switch ( src.Id )
    {
    case RequestId::HookDriver:
    case RequestId::UnhookDriver:
    case RequestId::EnableMonitoring:
    case RequestId::DisableMonitoring:
        dst["driver_name"] = CFB::Utils::ToString(src.DriverName);
        break;

    default:
        break;
    }
}

void
from_json(const json& js, DriverRequest& dst)
{
    dst.Id = js["id"];
    switch ( dst.Id )
    {
    case RequestId::HookDriver:
    case RequestId::UnhookDriver:
    case RequestId::EnableMonitoring:
    case RequestId::DisableMonitoring:
        dst.DriverName = CFB::Utils::ToWideString(js["driver_name"].get<std::string>());
        break;

    default:
        break;
    }
}

std::string
ToString(CFB::Comms::RequestId id)
{
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
        CaseToString(RequestId::StoreTestCase);
        CaseToString(RequestId::EnumerateDriverObject);
        CaseToString(RequestId::EnumerateDeviceObject);
    }
#undef CaseToString

    return "RequestId::Unknown";
}
} // namespace CFB::Comms
#endif // CFB_KERNEL_DRIVER
