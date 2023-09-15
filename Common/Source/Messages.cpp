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

    case RequestId::GetPendingIrp:
        dst["number_of_irp"] = src.NumberOfIrp;
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

    case RequestId::GetPendingIrp:
        dst.NumberOfIrp = js["number_of_irp"];
        break;

    default:
        break;
    }
}

} // namespace CFB::Comms
#endif // CFB_KERNEL_DRIVER
