#include "GuiUtils.hpp"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
std::optional<std::filesystem::path>
OpenFileWindows(std::string_view const& Filter)
{
    std::string filestr;
    filestr.resize(MAX_PATH);

    OPENFILENAMEA ofn {};
    ofn.lStructSize  = sizeof(OPENFILENAME);
    ofn.hwndOwner    = ::GetActiveWindow();
    ofn.lpstrFile    = filestr.data();
    ofn.nMaxFile     = filestr.size();
    ofn.lpstrFilter  = Filter.data();
    ofn.nFilterIndex = 1;
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if ( ::GetOpenFileNameA(&ofn) != TRUE )
    {
        return std::nullopt;
    }
    filestr.resize(::strlen(ofn.lpstrFile));
    return std::filesystem::path(filestr);
}

std::optional<std::filesystem::path>
SaveFileWindows(std::string_view const& Filter)
{
    std::string filestr;
    filestr.resize(MAX_PATH);

    OPENFILENAMEA ofn {};
    ofn.lStructSize  = sizeof(OPENFILENAME);
    ofn.hwndOwner    = ::GetActiveWindow();
    ofn.lpstrFile    = filestr.data();
    ofn.nMaxFile     = filestr.size();
    ofn.lpstrFilter  = Filter.data();
    ofn.nFilterIndex = 1;
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if ( ::GetSaveFileNameA(&ofn) != TRUE )
    {
        return std::nullopt;
    }
    filestr.resize(::strlen(ofn.lpstrFile));
    return std::filesystem::path(filestr);
}
#endif // _WIN32

namespace CFB::GUI::Utils
{

std::optional<std::filesystem::path>
FileManager::OpenFile(std::string_view const& Filter)
{
#ifdef _WIN32
    return OpenFileWindows(Filter);
#else
    return std::nullopt;
#endif
}


std::optional<std::filesystem::path>
FileManager::SaveFile(std::string_view const& Filter)
{
#ifdef _WIN32
    return SaveFileWindows(Filter);
#else
    return std::nullopt;
#endif
}


} // namespace CFB::GUI::Utils
