#include "App.hpp"

#include <algorithm>
#include <array>
#include <codecvt>
#include <fstream>
#include <json.hpp>
#include <locale>
#include <optional>
#include <ranges>

#include "Addons/imgui_hexeditor.h"
#include "Helpers.hpp"
#include "Utils.hpp"
#include "imgui.h"

static bool* p_open                       = nullptr;
static bool opt_fullscreen                = true;
static bool opt_padding                   = false;
static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

namespace views = std::ranges::views;
using json      = nlohmann::json;

static std::unordered_map<std::string_view, bool> Windows = {
    {"SessionInfo", true},
    {"IrpTable", true},
    {"IrpFactory", false},
    {"IrpDetail", false},
    {"Settings", false},
    {"Demo", false},
};


namespace CFB::GUI::App
{

Context Globals;

void
Context::LoadIrpsFromFile(std::filesystem::path const& JsonFile)
{
    if ( !std::filesystem::exists(JsonFile) )
    {
        return;
    }

    std::ifstream ifs(JsonFile);
    json j       = json::parse(ifs);
    auto NewIrps = j.get<std::vector<CFB::Comms::CapturedIrp>>();
    std::copy(NewIrps.begin(), NewIrps.end(), std::back_inserter(CapturedIrps));
}

void
Context::SaveIrpsToFile(std::filesystem::path const& JsonFile)
{
    json j = Globals.CapturedIrps;
    std::ofstream file(JsonFile);
    file << j.dump();
}

void
RenderSettingsWindow()
{
    ImGui::Begin("Settings");

    if ( ImGui::BeginChild("SettingsChild1") )
    {
        ImGui::SetItemDefaultFocus();

        ImGui::InputText("CFB Broker Host", Globals.Target.Host.data(), Globals.Target.Host.capacity());
        ImGui::InputInt("CFB Broker Port", (int*)&Globals.Target.Port, 0, 0, ImGuiInputTextFlags_CharsDecimal);
        const std::array<const char*, 1> fonts = {
            Globals.SelectedFontPath.c_str(),
        };
        static int current_font_index = 0;
        ImGui::Combo("Font", &current_font_index, fonts.data(), fonts.size());

        ImGui::Separator();

        if ( ImGui::Button("OK") )
        {
            Windows["Settings"] = false;
        }

        ImGui::EndChild();
    }
    ImGui::End();
}

void
PrepareMenubar()
{
    if ( ImGui::BeginMenuBar() )
    {
        if ( ImGui::BeginMenu("File") )
        {
            if ( ImGui::MenuItem("Save IRPs to file") )
            {
                auto fpath = CFB::GUI::Utils::FileManager::SaveFile("JSON Files\0*.json\0\0");
                if ( fpath )
                {
                    Globals.SaveIrpsToFile(fpath.value());
                }
            }

            if ( ImGui::MenuItem("Load IRPs from file") )
            {
                auto fpath = CFB::GUI::Utils::FileManager::OpenFile("JSON Files\0*.json\0\0");
                if ( fpath )
                {
                    Globals.LoadIrpsFromFile(fpath.value());
                }
            }

            ImGui::Separator();

            if ( ImGui::MenuItem("Restart") )
            {
                Globals.KeepRunning = false;
                Globals.Restart     = true;
            }

            if ( ImGui::MenuItem("Quit") )
            {
                Globals.KeepRunning = false;
                Globals.Restart     = false;
            }

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu("Window") )
        {
            for ( auto const& WindowName : std::views::keys(Windows) )
            {
                if ( ImGui::MenuItem(WindowName.data(), nullptr, Windows[WindowName]) )
                {
                    Windows[WindowName] ^= true;
                }
            }

            ImGui::EndMenu();
        }

        if ( ImGui::BeginMenu("Help") )
        {
            // TODO finish
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

// move to helpers
static inline void
InputHex(const char* label, void* value, int type)
{
    const static auto TEXT_WIDTH = ImGui::CalcTextSize("A").x;
    const float INPUT_HEX_WIDTH  = TEXT_WIDTH * 20;
    const char* fmt              = nullptr;
    switch ( type )
    {
    case ImGuiDataType_U8:
        fmt = "0x%02X";
        break;
    case ImGuiDataType_U16:
        fmt = "0x%04X";
        break;
    case ImGuiDataType_U32:
        fmt = "0x%08X";
        break;
    case ImGuiDataType_U64:
        fmt = "0x%16X";
        break;
    default:
        fmt = "0x%08X";
        break;
    }

    ImGui::PushItemWidth(INPUT_HEX_WIDTH);
    ImGui::InputScalar(
        label,
        type,
        value,
        nullptr,
        nullptr,
        fmt,
        ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();
}

static inline void
InputHexU8(const char* label, void* value)
{
    InputHex(label, value, ImGuiDataType_U8);
}

static inline void
InputHexU16(const char* label, void* value)
{
    InputHex(label, value, ImGuiDataType_U16);
}

static inline void
InputHexU32(const char* label, void* value)
{
    InputHex(label, value, ImGuiDataType_U32);
}

static inline void
InputHexU64(const char* label, void* value)
{
    InputHex(label, value, ImGuiDataType_U64);
}

void
PrepareDockSpace()
{
    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if ( opt_fullscreen )
    {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    else
    {
        dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
    }

    // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
    // and handle the pass-thru hole, so we ask Begin() to not render a background.
    if ( dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode )
        window_flags |= ImGuiWindowFlags_NoBackground;

    // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
    // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
    // all active windows docked into it will lose their parent and become undocked.
    // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
    // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
    if ( !opt_padding )
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("DockSpace1", p_open, window_flags);
    if ( !opt_padding )
        ImGui::PopStyleVar();

    if ( opt_fullscreen )
        ImGui::PopStyleVar(2);

    // Submit the DockSpace
    ImGuiIO& io = ImGui::GetIO();
    if ( io.ConfigFlags & ImGuiConfigFlags_DockingEnable )
    {
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }

    PrepareMenubar();

    ImGui::End();
}

void
RenderSessionInfoWindow()
{
    ImGui::Begin("SessionInfo");

    ImGui::Text("Viewing %llu IRPs", Globals.CapturedIrps.size());

    ImGui::Separator();

    ImGui::Text("Target: %s:%u", Globals.Target.Host.c_str(), Globals.Target.Port);
    ImGui::Text("Connected: %s", boolstr(Globals.Target.IsConnected));

    bool NewState = Globals.Target.IsConnected;
    CFB::GUI::Helpers::ToggleButton(Globals.Target.IsConnected ? "Disconnect" : "Connect", &NewState);
    if ( NewState != Globals.Target.IsConnected )
    {
        Globals.Target.IsConnected ? Globals.Target.Disconnect() : Globals.Target.Connect();
    }

    if ( Globals.Target.IsConnected )
    {
        ImGui::Text(
            "Hooked Drivers: %llu",
            std::count_if(
                Globals.Drivers.cbegin(),
                Globals.Drivers.cend(),
                [](std::pair<std::string, std::pair<bool, bool>> const& Entry)
                {
                    return Entry.second.first == true;
                }));
        ImGui::Text(
            "Monitored Drivers: %llu",
            std::count_if(
                Globals.Drivers.cbegin(),
                Globals.Drivers.cend(),
                [](std::pair<std::string, std::pair<bool, bool>> const& Entry)
                {
                    return Entry.second.second == true;
                }));
        ImGui::Text("Driver Found: %llu", Globals.Drivers.size());

        if ( Globals.RefreshingDrivers )
        {
            CFB::GUI::Helpers::Spinner("DriverRefresh##spinner", 7, 3, ImGui::GetColorU32(ImGuiCol_Button));
        }
        else if ( ImGui::Button("Refresh##RefreshDriver") )
        {
            Globals.RefreshDriverList();
        }

        static ImGuiTextFilter DriverFilter;
        DriverFilter.Draw();

        auto DriverView = Globals.Drivers | std::views::filter(
                                                [](auto const& Entry) -> bool
                                                {
                                                    return (DriverFilter.PassFilter(Entry.first.c_str()));
                                                });

        for ( auto& [Driver, Flags] : DriverView )
        {
            if ( ImGui::TreeNode(Driver.c_str()) )
            {
                bool DoHook = Flags.first;
                if ( ImGui::Checkbox("Is Hooked", &DoHook) )
                {
                    DoHook ? Globals.EnableDriver(Driver) : Globals.DisableDriver(Driver);
                }

                if ( DoHook )
                {
                    bool DoCapture = Flags.second;
                    if ( ImGui::Checkbox("Is Capturing", &DoCapture) )
                    {
                        DoCapture ? Globals.EnableMonitoring(Driver) : Globals.DisableMonitoring(Driver);
                    }
                }
                ImGui::TreePop();
            }
        }
    }

    ImGui::End();
}

void
RenderIrpFactoryWindow()
{
    ImGui::Begin("IrpFactory");

    static std::string DevicePath;
    DevicePath.resize(MAX_PATH);
    static i32 IoctlCode, InputBufferLength, OutputBufferLength, StatusCode;

    ImGui::InputText("Device Path", DevicePath.data(), DevicePath.size());
    ImGui::InputInt("IOCTL Code", &IoctlCode);

    ImGui::Separator();

    ImGui::InputInt("Input Buffer Length", &InputBufferLength);
    ImGui::InputInt("Output Buffer Length", &OutputBufferLength);
    ImGui::InputInt("Status Code", &StatusCode);

    ImGui::Separator();

    ImGui::Button("Send IRP");
    ImGui::SameLine();
    ImGui::Button("Clear");

    ImGui::End();
}

void
RenderIrpDetailWindow(CFB::Comms::CapturedIrp& Irp)
{
    const static auto TEXT_WIDTH = ImGui::CalcTextSize("A").x;
    const float INPUT_HEX_WIDTH  = TEXT_WIDTH * 20;

    static MemoryEditor InputBufferView, OutputBufferView;

    ImGui::Begin("IrpDetail");
    std::string DevicePath        = CFB::Utils::ToString(Irp.Header.DeviceName);
    std::string DriverPath        = CFB::Utils::ToString(Irp.Header.DriverName);
    std::string ProcessName       = CFB::Utils::ToString(Irp.Header.ProcessName);
    std::vector<u8>& InputBuffer  = Irp.InputBuffer;
    std::vector<u8>& OutputBuffer = Irp.OutputBuffer;

    ImGui::InputText("Driver Path", DriverPath.data(), DriverPath.size(), ImGuiInputTextFlags_ReadOnly);
    ImGui::InputText("Device Path", DevicePath.data(), DevicePath.size(), ImGuiInputTextFlags_ReadOnly);
    ImGui::InputText("Process Name", ProcessName.data(), ProcessName.size(), ImGuiInputTextFlags_ReadOnly);

    InputHexU32("IOCTL Code", (void*)&Irp.Header.IoctlCode);
    ImGui::SameLine();
    InputHexU32("Status Code", (void*)&Irp.Header.Status);

    ImGui::PushItemWidth(INPUT_HEX_WIDTH);
    ImGui::InputInt("PID", (int*)&Irp.Header.Pid, 0, 0, ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    ImGui::InputInt("TID", (int*)&Irp.Header.Tid, 0, 0, ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();

    InputHexU8("Major Function", (void*)&Irp.Header.MajorFunction);
    ImGui::SameLine();
    InputHexU8("Minor Function", (void*)&Irp.Header.MinorFunction);

    InputHexU32("Input Buffer Length", (void*)&Irp.Header.InputBufferLength);
    ImGui::SameLine();
    InputHexU32("Output Buffer Length", (void*)&Irp.Header.OutputBufferLength);
    ImGui::End();

    if ( !InputBuffer.empty() )
    {
        InputBufferView.DrawWindow("Input Buffer Viewer", InputBuffer.data(), InputBuffer.size());
    }

    if ( !OutputBuffer.empty() )
    {
        OutputBufferView.DrawWindow("Output Buffer Viewer", OutputBuffer.data(), OutputBuffer.size());
    }
}

void
RenderIrpTableWindow()
{
    ImGui::Begin("IrpTable", nullptr, ImGuiWindowFlags_NoTitleBar);

    static ImGuiTextFilter filter;
    filter.Draw();

    ImGui::Separator();

    // const float TEXT_BASE_WIDTH  = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    const ImGuiTableFlags IrpTableFlags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
        ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti |
        ImGuiTableFlags_NoBordersInBodyUntilResize;

    auto const& Irps                                             = Globals.CapturedIrps;
    const usize ColumnNumber                                     = 14;
    const std::array<std::string_view, ColumnNumber> ColumnNames = {
        "TimeStamp",
        "Driver Name",
        "Device Name",
        "IRQ Level",
        "Type",
        "Major Function",
        "Minor Function",
        "IOCTL Code",
        "Pid",
        "Tid",
        "Process Name",
        "Status",
        "Input Length",
        "Output Length",
    };

    auto IrpsView = Irps | std::views::filter(
                               [](CFB::Comms::CapturedIrp const& Irp) -> bool
                               {
                                   auto const IrpAsString = CFB::Comms::ToString(Irp);
                                   return (filter.PassFilter(IrpAsString.c_str()));
                               });

    static std::optional<CFB::Comms::CapturedIrp> SelectedIrp = std::nullopt;

    if ( ImGui::BeginTable("IrpTable", ColumnNumber, IrpTableFlags, ImVec2(0.0f, TEXT_BASE_HEIGHT * 8)) )
    {
        //
        // Header row
        //
        ImGui::TableSetupScrollFreeze(0, 1);
        for ( auto const& HeaderLabel : ColumnNames )
        {
            int flags = ImGuiTableColumnFlags_WidthStretch;
            if ( HeaderLabel == "TimeStamp" )
            {
                flags |= ImGuiTableColumnFlags_DefaultSort;
            }
            ImGui::TableSetupColumn(HeaderLabel.data(), flags);
        }
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        //
        // Populate data
        //
        usize RowIndex = 0;
        for ( auto const& Irp : IrpsView )
        {
            bool selected = false;
            ImGui::TableNextRow();

            //
            // Populate row fields
            //
            ImGui::TableNextColumn();
            std::string label = std::to_string(Irp.Header.TimeStamp);
            label += "##" + std::to_string(RowIndex);
            ImGui::Selectable(label.c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns);

            ImGui::TableNextColumn();
            ImGui::Text("%S", const_cast<wchar_t*>(Irp.Header.DriverName));

            ImGui::TableNextColumn();
            ImGui::Text("%S", const_cast<wchar_t*>(Irp.Header.DeviceName));

            ImGui::TableNextColumn();
            // TODO use helper
            ImGui::Text(
                "%s",
                Irp.Header.Irql == 0 ?
                    "PASSIVE_LEVEL" :
                    (Irp.Header.Irql == 1 ? "APC_LEVEL" : (Irp.Header.Irql == 2 ? "DISPATCH_LEVEL" : "HIGH")));

            ImGui::TableNextColumn();
            // TODO use helper
            ImGui::Text("%s", Irp.Header.Type == 0 ? "IRP" : (Irp.Header.Type == 1 ? "FastIRP" : "Unknown"));

            ImGui::TableNextColumn();
            ImGui::Text("%s", CFB::Utils::IrpMajorToString(Irp.Header.MajorFunction).c_str());

            ImGui::TableNextColumn();
            ImGui::Text("%u", Irp.Header.MinorFunction);

            ImGui::TableNextColumn();
            ImGui::Text("0x%08X", Irp.Header.IoctlCode);

            ImGui::TableNextColumn();
            ImGui::Text("%lu", Irp.Header.Pid);

            ImGui::TableNextColumn();
            ImGui::Text("%lu", Irp.Header.Tid);

            ImGui::TableNextColumn();
            ImGui::Text("%S", const_cast<wchar_t*>(Irp.Header.ProcessName));

            ImGui::TableNextColumn();
            ImGui::Text("0x%08X", Irp.Header.Status);

            ImGui::TableNextColumn();
            ImGui::Text("%lu", Irp.Header.InputBufferLength);

            ImGui::TableNextColumn();
            ImGui::Text("%lu", Irp.Header.OutputBufferLength);

            if ( selected )
            {
                SelectedIrp = Irp;
            }

            RowIndex++;
        }

        if ( SelectedIrp )
        {
            RenderIrpDetailWindow(SelectedIrp.value());
        }

        ImGui::EndTable();
    }
    ImGui::End();
} // namespace CFB::GUI::App

void
RenderUI()
{
    //
    // layout
    //
    PrepareDockSpace();

    //
    // main windows
    //
    if ( Windows["SessionInfo"] )
    {
        RenderSessionInfoWindow();
    }

    if ( Windows["IrpTable"] )
    {
        RenderIrpTableWindow();
    }

    if ( Windows["IrpFactory"] )
    {
        RenderIrpFactoryWindow();
    }

    //
    // extra stuff
    //
    if ( Windows["Settings"] )
    {
        RenderSettingsWindow();
    }

    if ( Windows["Demo"] )
    {
        ImGui::ShowDemoWindow();
    }
}

} // namespace CFB::GUI::App
