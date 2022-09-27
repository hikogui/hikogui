// Copyright Jens A. Koch 2021.
// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "win32_headers.hpp"

#include "URL.hpp"
#include "strings.hpp"
#include "utility.hpp"
#include "url_parser.hpp"
#include "log.hpp"
#include "metadata.hpp"
#include <regex>

namespace hi::inline v1 {

/*! Convenience function for SHGetKnownFolderPath().
 *  Retrieves a full path of a known folder identified by the folder's KNOWNFOLDERID.
 *  See https://docs.microsoft.com/en-us/windows/win32/shell/knownfolderid#constants
 *
 * \param KNOWNFOLDERID folder_id.
 * \return The URL of the folder.
 */
static URL get_folder_by_id(const KNOWNFOLDERID &folder_id) noexcept
{
    PWSTR path = nullptr;
    if (SHGetKnownFolderPath(folder_id, 0, nullptr, &path) != S_OK) {
        hi_log_fatal("Could not get known folder path.");
    }
    URL folder = URL{std::filesystem::path{path} / ""};
    CoTaskMemFree(path);
    return folder;
}

URL URL::url_from_executable_file() noexcept
{
    std::wstring module_path;
    auto buffer_size = MAX_PATH; // initial default value = 256
    // iterative buffer resizing to max value of 32768 (256*2^7)
    for (std::size_t i = 0; i < 7; ++i) {
        module_path.resize(buffer_size);
        auto chars = GetModuleFileNameW(nullptr, &module_path[0], buffer_size);
        if (chars < module_path.length()) {
            module_path.resize(chars);
            return URL{std::filesystem::path{module_path}};
        } else {
            buffer_size *= 2;
        }
    }
    hi_log_fatal("Could not get executable path. It exceeds the buffer length of 32768 chars.");
}

URL URL::url_from_resource_directory() noexcept
{
    // Resource path, is the same directory as where the executable lives.
    return url_from_executable_directory() / "resources";
}

URL URL::url_from_application_data_directory() noexcept
{
    // FOLDERID_LocalAppData has the default path: %LOCALAPPDATA% (%USERPROFILE%\AppData\Local)
    if (metadata::application().vendor.empty()) {
        return get_folder_by_id(FOLDERID_LocalAppData) / metadata::application().display_name;

    } else {
        return get_folder_by_id(FOLDERID_LocalAppData) / metadata::application().vendor / metadata::application().display_name;
    }
}

URL URL::url_from_system_font_directory() noexcept
{
    // FOLDERID_Fonts has the default path: %windir%\Fonts
    return get_folder_by_id(FOLDERID_Fonts);
}

URL URL::url_from_application_preferences_file() noexcept
{
    return URL::url_from_application_data_directory() / "preferences.json";
}

} // namespace hi::inline v1
