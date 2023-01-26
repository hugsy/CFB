#pragma once


#include <filesystem>
#include <string>
#include <vector>

#include "Common.hpp"
#include "Comms.hpp"

namespace CFB::GUI::App
{

struct Context
{
    Context() :
        Restart {false},
        IsCapturing {false},
        KeepRunning {true},
        ShowSettingPopup {false},
        Target {"tcp://192.168.57.1:1337"},
        Connected {false},
        SelectedFontPath {"C:\\Windows\\Fonts\\SegoeUI.ttf"},
        FontSize {16.0f},
        HookedDrivers {},
        CapturedIrps {}
    {
        Target.resize(1024);
    }

    bool Restart;
    bool KeepRunning;
    bool ShowSettingPopup;
    bool IsCapturing;
    std::string Target;
    bool Connected;
    std::string SelectedFontPath;
    float FontSize;
    std::vector<std::string> HookedDrivers;
    std::vector<CFB::Comms::CapturedIrp> CapturedIrps;

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
};


///
///@brief Main function to render CFB
///
///
void
RenderUI();

extern Context Globals;

} // namespace CFB::GUI::App
