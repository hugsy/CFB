#include "Helpers.hpp"

#include <cmath>

#include "imgui_internal.h"

void
CFB::GUI::Helpers::ToggleButton(const char* str_id, bool* v)
{
    ImVec2 p              = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width  = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if ( ImGui::IsItemClicked() )
        *v = !*v;

    float t = *v ? 1.0f : 0.0f;

    ImGuiContext& g  = *GImGui;
    float ANIM_SPEED = 0.08f;
    if ( g.LastActiveId == g.CurrentWindow->GetID(str_id) ) // && g.LastActiveIdTimer < ANIM_SPEED)
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t            = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg;
    if ( ImGui::IsItemHovered() )
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
    else
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(
        ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius),
        radius - 1.5f,
        IM_COL32(255, 255, 255, 255));
}


bool
CFB::GUI::Helpers::BufferingBar(
    const char* label,
    float value,
    const ImVec2& size_arg,
    const ImU32& bg_col,
    const ImU32& fg_col)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if ( window->SkipItems )
        return false;

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id        = window->GetID(label);

    ImVec2 pos  = window->DC.CursorPos;
    ImVec2 size = size_arg;
    size.x -= style.FramePadding.x * 2;

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(bb, style.FramePadding.y);
    if ( !ImGui::ItemAdd(bb, id) )
        return false;

    // Render
    const float circleStart = size.x * 0.7f;
    const float circleEnd   = size.x;
    const float circleWidth = circleEnd - circleStart;

    window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
    window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart * value, bb.Max.y), fg_col);

    const float t     = g.Time;
    const float r     = size.y / 2;
    const float speed = 1.5f;

    const float a = speed * 0;
    const float b = speed * 0.333f;
    const float c = speed * 0.666f;

    const float o1 = (circleWidth + r) * (t + a - speed * (int)((t + a) / speed)) / speed;
    const float o2 = (circleWidth + r) * (t + b - speed * (int)((t + b) / speed)) / speed;
    const float o3 = (circleWidth + r) * (t + c - speed * (int)((t + c) / speed)) / speed;

    window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
    window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
    window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);

    return true;
}

bool
CFB::GUI::Helpers::Spinner(const char* label, float radius, int thickness, const ImU32& color)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if ( window->SkipItems )
        return false;

    ImGuiContext& g         = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id        = window->GetID(label);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size((radius)*2, (radius + style.FramePadding.y) * 2);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(bb, style.FramePadding.y);
    if ( !ImGui::ItemAdd(bb, id) )
        return false;

    // Render
    window->DrawList->PathClear();

    int num_segments = 30;
    int start        = std::abs(ImSin(g.Time * 1.8f) * (num_segments - 5));

    const float a_min = IM_PI * 2.0f * ((float)start) / (float)num_segments;
    const float a_max = IM_PI * 2.0f * ((float)num_segments - 3) / (float)num_segments;

    const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

    for ( int i = 0; i < num_segments; i++ )
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        window->DrawList->PathLineTo(
            ImVec2(centre.x + ImCos(a + g.Time * 8) * radius, centre.y + ImSin(a + g.Time * 8) * radius));
    }

    window->DrawList->PathStroke(color, false, thickness);

    return true;
}
