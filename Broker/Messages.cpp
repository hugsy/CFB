#include "Messages.hpp"

#include <codecvt>
#include <locale>


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
    case RequestId::EnableDriver:
    case RequestId::DisableDriver:
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
        dst["driver_name"] = cvt.to_bytes(src.DriverName);
        break;
    }

    default:
        break;
    }
}

void
from_json(const json& src, DriverRequest& dst)
{
    src.at("id").get_to(dst.Id);

    switch ( dst.Id )
    {
    case RequestId::HookDriver:
    case RequestId::UnhookDriver:
    case RequestId::EnableDriver:
    case RequestId::DisableDriver:
    {
        std::string driver_name;
        src.at("driver_name").get_to(driver_name);
        std::wstring_convert<std::codecvt_utf8<wchar_t>> cvt;
        dst.DriverName = cvt.from_bytes(driver_name);
        break;
    }

    default:
        break;
    }
}

} // namespace CFB::Comms
