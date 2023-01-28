#pragma once

// clang-format off
#include "Common.hpp"
#include "Comms.hpp"
#include "Messages.hpp"
#include "json.hpp"

#include <chrono>
#include <filesystem>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Network.hpp"
// clang-format on


using json = nlohmann::json;
using namespace std::literals::chrono_literals;

namespace CFB::GUI::App
{

class Context
{
public:
    Context() :
        Restart {false},
        KeepRunning {true},
        ShowSettingPopup {false},
        SelectedFontPath {"C:\\Windows\\Fonts\\SegoeUI.ttf"},
        FontSize {16.0f},
        Drivers {},
        Target {},
        RefreshingDrivers {false},
        CapturedIrps {}
    {
        Target.Host.reserve(1024);
        Target.Host.assign("192.168.57.87");
        Target.Port        = 1337;
        Target.IsConnected = false;
    }

    ~Context()
    {
        if ( Target.IsConnected )
        {
            Target.Disconnect();
        }
    }

    bool Restart;
    bool KeepRunning;
    bool ShowSettingPopup;

    std::string SelectedFontPath;
    float FontSize;
    std::unordered_map<std::string, std::pair<bool, bool>> Drivers;
    std::vector<CFB::Comms::CapturedIrp> CapturedIrps;
    bool RefreshingDrivers;
    Target Target;

    ///
    ///@brief
    ///
    ///@param JsonFile
    ///
    void
    LoadIrpsFromFile(std::filesystem::path const& JsonFile);

    ///
    ///@brief
    ///
    ///@param JsonFile
    ///
    void
    SaveIrpsToFile(std::filesystem::path const& JsonFile);

    ///
    ///@brief
    ///
    ///@return true
    ///@return false
    ///
    bool
    RefreshDriverList()
    {
        bool res = false;
        std::jthread thr {[this, &res]
                          {
                              Drivers.clear();

                              RefreshingDrivers = true;

                              CFB::Comms::DriverRequest req;
                              req.Id = CFB::Comms::RequestId::EnumerateDriverObject;

                              auto rep = SendCommand(req);
                              if ( !rep || rep.value()["error_code"] != 0 )
                              {
                                  res = false;
                                  return;
                              }

                              for ( std::string const& DriverPath : rep.value()["body"]["body"] )
                              {
                                  Drivers[DriverPath] = {false, false};
                              }

                              RefreshingDrivers = false;
                              res               = true;
                              return;
                          }};
        thr.detach();
        return res;
    }

    bool
    EnableMonitoring(std::string const& DriverName)
    {
        if ( Drivers[DriverName].second )
        {
            return true;
        }

        CFB::Comms::DriverRequest req;
        req.Id         = CFB::Comms::RequestId::EnableMonitoring;
        req.DriverName = CFB::Utils::ToWideString(DriverName);
        auto rep       = SendCommand(req);
        if ( !rep || rep.value()["error_code"] != 0 )
        {
            return false;
        }

        if ( rep.value()["body"]["success"] == false )
        {
            return false;
        }

        Drivers[DriverName].second = true;
        return true;
    }

    bool
    DisableMonitoring(std::string const& DriverName)
    {
        if ( !Drivers[DriverName].second )
        {
            return true;
        }

        CFB::Comms::DriverRequest req;
        req.Id         = CFB::Comms::RequestId::DisableMonitoring;
        req.DriverName = CFB::Utils::ToWideString(DriverName);
        auto rep       = SendCommand(req);
        if ( !rep || rep.value()["error_code"] != 0 )
        {
            return false;
        }

        if ( !rep.value()["body"]["success"] )
        {
            return false;
        }
        Drivers[DriverName].second = false;
        return true;
    }

    bool
    EnableDriver(std::string const& DriverName)
    {
        if ( Drivers[DriverName].first )
        {
            return true;
        }

        bool res = false;
        CFB::Comms::DriverRequest req;
        req.Id         = CFB::Comms::RequestId::HookDriver;
        req.DriverName = CFB::Utils::ToWideString(DriverName);

        auto rep = SendCommand(req);
        if ( !rep || rep.value()["error_code"] != 0 )
        {
            res = false;
            return res;
        }

        res = rep.value()["body"]["success"] == true;
        if ( res )
        {
            Drivers[DriverName].first = true;
        }
        return res;
    }

    bool
    DisableDriver(std::string const& DriverName)
    {
        if ( !Drivers[DriverName].first )
        {
            return true;
        }

        bool res = false;
        CFB::Comms::DriverRequest req;
        req.Id         = CFB::Comms::RequestId::UnhookDriver;
        req.DriverName = CFB::Utils::ToWideString(DriverName);

        auto rep = SendCommand(req);
        if ( !rep || rep.value()["error_code"] != 0 )
        {
            res = false;
            return res;
        }

        res = rep.value()["body"]["success"] == true;
        if ( res )
        {
            Drivers[DriverName].first = false;
        }
        return res;
    }

private:
    std::optional<json>
    SendCommand(CFB::Comms::DriverRequest const& Command)
    {
        // Request
        {
            json req;
            CFB::Comms::to_json(req, Command);
            if ( !Target.Send(req.dump()) )
            {
                return std::nullopt;
            }
        }

        // Response
        {
            auto Body = Target.Receive<std::string>(4096);
            if ( Body.size() == 0 )
            {
                return std::nullopt;
            }

            return json::parse(Body);
        }
    }
};


///
///@brief Main function to render CFB
///
///
void
RenderUI();


///
///@brief
///
///
extern Context Globals;

} // namespace CFB::GUI::App
