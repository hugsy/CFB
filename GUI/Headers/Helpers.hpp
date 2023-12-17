#pragma once

#include <imgui.h>

namespace CFB::GUI::Helpers
{

///
///@brief From https://github.com/ocornut/imgui/issues/1537#issuecomment-355569554
///
///@param str_id
///@param v
///
bool
ToggleButton(const char* str_id, bool* v);


///
///@brief From https://github.com/ocornut/imgui/issues/1901#issue-335266223
///
///@param label
///@param value
///@param size_arg
///@param bg_col
///@param fg_col
///@return true
///@return false
///
bool
BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);


///
///@brief From https://github.com/ocornut/imgui/issues/1901#issue-335266223
///
///@param label
///@param radius
///@param thickness
///@param color
///@return true
///@return false
///
bool
Spinner(const char* label, float radius, int thickness, const ImU32& color);

} // namespace CFB::GUI::Helpers
