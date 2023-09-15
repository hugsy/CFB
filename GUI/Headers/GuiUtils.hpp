#pragma once

#include <filesystem>
#include <optional>
#include <string_view>


namespace CFB::GUI::Utils
{

class FileManager
{
public:
    ///
    ///@brief
    ///
    ///@param Filter
    ///@return std::optional<std::filesystem::path>
    ///
    static std::optional<std::filesystem::path>
    OpenFile(std::string_view const& Filter);

    ///
    ///@brief
    ///
    ///@param Filter
    ///@return std::optional<std::filesystem::path>
    ///
    static std::optional<std::filesystem::path>
    SaveFile(std::string_view const& Filter);
};
} // namespace CFB::GUI::Utils
