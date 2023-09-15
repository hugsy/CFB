#define CATCH_CONFIG_MAIN
#define NS "CFB::Comms"

// clang-format off
#include <catch.hpp>
#include <nlohmann/json.hpp>

#include "Comms.hpp"
// clang-format on

using json = nlohmann::json;
using namespace nlohmann::literals;

TEST_CASE("Common/Communication", NS)
{
    SECTION("IRP from JS")
    {
        json j1 = R"(
[
    {
        "Header":
        {
            "DeviceName":"\\Device\\HackSysExtremeVulnerableDriver",
            "DriverName":"\\driver\\HEVD",
            "InputBufferLength":0,
            "IoctlCode":0,
            "Irql":0,
            "MajorFunction":0,
            "MinorFunction":0,
            "OutputBufferLength":0,
            "Pid":6596,
            "ProcessName": "DriverClient.e",
            "Status":0,
            "Tid":8056,
            "TimeStamp":133194954083127683,
            "Type":0
        },
        "InputBuffer":[],
        "OutputBuffer":[]
    },
    {
        "Header":
        {
            "DeviceName":"\\Device\\HackSysExtremeVulnerableDriver",
            "DriverName":"\\driver\\HEVD",
            "InputBufferLength":32,
            "IoctlCode":2236419,
            "Irql":0,
            "MajorFunction":14,
            "MinorFunction":0,
            "OutputBufferLength":0,
            "Pid":6596,
            "ProcessName": "DriverClient.e",
            "Status":0,
            "Tid":8056,
            "TimeStamp":133194954083127682,
            "Type":0
        },
        "InputBuffer":[65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65,65],
        "OutputBuffer":[]
    }
]
)"_json;

        for ( auto const& x : j1 )
        {
            auto const& i = x.get<CFB::Comms::CapturedIrp>();
            CHECK(i.InputBuffer.size() == i.Header.InputBufferLength);
            CHECK(i.OutputBuffer.size() == i.Header.OutputBufferLength);
        }

        //
        // Too long device, driver or process name -> trimmed
        //
        json j2 = R"(
{
    "Header":
    {
        "DeviceName":"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        "DriverName":"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB",
        "InputBufferLength":0,
        "IoctlCode":0,
        "Irql":0,
        "MajorFunction":0,
        "MinorFunction":0,
        "OutputBufferLength":0,
        "Pid":6596,
        "ProcessName": "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC",
        "Status":0,
        "Tid":8056,
        "TimeStamp":133194954083127683,
        "Type":0
    },
    "InputBuffer":[],
    "OutputBuffer":[]
}
    )"_json;

        auto const& i = j2.get<CFB::Comms::CapturedIrp>();
        CHECK(::wcslen(i.Header.DeviceName) == (CFB_DRIVER_MAX_PATH - 1));
        CHECK(::wcslen(i.Header.DriverName) == (CFB_DRIVER_MAX_PATH - 1));
        CHECK(::wcslen(i.Header.ProcessName) == (CFB_DRIVER_MAX_PATH - 1));
    }
}
