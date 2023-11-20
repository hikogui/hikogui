// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"

#include "path_location_intf.hpp"
#include "../metadata/metadata.hpp"
#include "../telemetry/telemetry.hpp"
#include "../utility/utility.hpp"
#include "../coroutine/coroutine.hpp"
#include "../char_maps/char_maps.hpp" // XXX #616
#include "../macros.hpp"
#include <filesystem>
#include <string>
#include <coroutine>

hi_export_module(hikogui.path.path_location : impl);

hi_export namespace hi::inline v1 {

/** Convenience function for SHGetKnownFolderPath().
 *  Retrieves a full path of a known folder identified by the folder's KNOWNFOLDERID.
 *  See https://docs.microsoft.com/en-us/windows/win32/shell/knownfolderid#constants
 *
 * @param KNOWNFOLDERID folder_id.
 * @return The path of the folder.
 */
hi_export [[nodiscard]] hi_inline std::filesystem::path get_path_by_id(const KNOWNFOLDERID& folder_id) noexcept
{
    PWSTR wpath = nullptr;
    if (SHGetKnownFolderPath(folder_id, 0, nullptr, &wpath) != S_OK) {
        hi_log_fatal("Could not get known folder path.");
    }
    hilet d = defer{[&] {
        CoTaskMemFree(wpath);
    }};

    return std::filesystem::path{wpath} / "";
}

hi_export [[nodiscard]] hi_inline std::filesystem::path executable_file() noexcept
{
    if (auto path = win32_GetModuleFileName()) {
        return *path;
    } else {
        hi_log_fatal("Could not get executable-file. {}", make_error_code(path.error()).message());
    }
}

hi_export [[nodiscard]] hi_inline std::filesystem::path data_dir() noexcept
{
    // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\"
    // FOLDERID_LocalAppData has the default path: %LOCALAPPDATA% (%USERPROFILE%\AppData\Local)
    hilet local_app_data = get_path_by_id(FOLDERID_LocalAppData);
    return local_app_data / get_application_vendor() / get_application_name() / "";
}

hi_export [[nodiscard]] hi_inline std::filesystem::path log_dir() noexcept
{
    // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\Log\"
    return data_dir() / "Log" / "";
}

hi_export [[nodiscard]] hi_inline std::filesystem::path preferences_file() noexcept
{
    // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\preferences.json"
    return data_dir() / "preferences.json";
}

hi_export [[nodiscard]] hi_inline generator<std::filesystem::path> resource_dirs() noexcept
{
    if (auto source_path = source_dir()) {
            // Fallback when the application is executed from its build directory.
            co_yield executable_dir() / "resources" / "";
            co_yield *source_path / "resources" / "";

            if (auto install_path = library_install_dir()) {
                co_yield *install_path / "resources" / "";

            } else {
                // Fallback when HikoGUI is also still in its build directory.
                co_yield library_source_dir() / "resources" / "";
                co_yield library_build_dir() / "resources" / "";
            }
        } else {
            co_yield executable_dir() / "resources" / "";
            co_yield data_dir() / "resources" / "";
        }
}

hi_export [[nodiscard]] hi_inline generator<std::filesystem::path> system_font_dirs() noexcept
{
    co_yield get_path_by_id(FOLDERID_Fonts);
}

hi_export [[nodiscard]] hi_inline generator<std::filesystem::path> font_dirs() noexcept
{
    for (hilet& path : resource_dirs()) {
        co_yield path;
    }
    for (hilet& path : system_font_dirs()) {
        co_yield path;
    }
}

hi_export [[nodiscard]] hi_inline generator<std::filesystem::path> theme_dirs() noexcept
{
    for (hilet& path : resource_dirs()) {
        co_yield path;
    }
}

} // namespace hi::inline v1
