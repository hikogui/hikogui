// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../win32_headers.hpp"

#include "path_location_intf.hpp"
#include "../metadata/metadata.hpp"
#include "../telemetry/telemetry.hpp"
#include "../utility/utility.hpp"
#include "../char_maps/char_maps.hpp" // XXX #616
#include "../win32/win32.hpp"
#include "../macros.hpp"
#include <filesystem>
#include <string>
#include <coroutine>

hi_export_module(hikogui.path.path_location : impl);

hi_export namespace hi {
inline namespace v1 {

[[nodiscard]] hi_inline std::filesystem::path executable_file() noexcept
{
    if (auto path = win32_GetModuleFileName()) {
        return *path;
    } else {
        hi_log_fatal("Could not get executable-file. {}", make_error_code(path.error()).message());
    }
}

[[nodiscard]] hi_inline std::filesystem::path data_dir()
{
    // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\"
    // FOLDERID_LocalAppData has the default path: %LOCALAPPDATA% (%USERPROFILE%\AppData\Local)
    if (auto path = win32_SHGetKnownFolderPath(FOLDERID_LocalAppData)) {
        return *path / get_application_vendor() / get_application_name() / "";
    } else {
        throw os_error{std::format("Could not get data directory: {}", std::error_code{path.error()}.message())};
    }
}

[[nodiscard]] hi_inline std::filesystem::path log_dir() noexcept
{
    // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\Log\"
    return data_dir() / "Log" / "";
}

[[nodiscard]] hi_inline std::filesystem::path preferences_file() noexcept
{
    // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\preferences.json"
    return data_dir() / "preferences.json";
}

[[nodiscard]] hi_inline generator<std::filesystem::path> resource_dirs() noexcept
{
    if (auto source_path = source_dir()) {
        // Fallback when the application is executed from its build directory.
        co_yield executable_dir() / "resources" / "";
        co_yield *source_path / "resources" / "";

        if (auto install_path = library_install_dir()) {
            co_yield *install_path / "resources" / "";

        } else {
            // Fallback when HikoGUI is also still in its build directory.
            if (auto library_source_dir_ = library_source_dir()) {
                co_yield (*library_source_dir_) / "resources" / "";
            }
            if (auto library_build_dir_ = library_build_dir()) {
                co_yield (*library_build_dir_) / "resources" / "";
            }
        }
    } else {
        co_yield executable_dir() / "resources" / "";
        co_yield data_dir() / "resources" / "";
    }
}

[[nodiscard]] hi_inline generator<std::filesystem::path> system_font_dirs() noexcept
{
    if (auto path = win32_SHGetKnownFolderPath(FOLDERID_Fonts)) {
        co_yield *path;
    }
}

[[nodiscard]] hi_inline generator<std::filesystem::path> font_dirs() noexcept
{
    for (auto const& path : resource_dirs()) {
        co_yield path;
    }
    for (auto const& path : system_font_dirs()) {
        co_yield path;
    }
}

[[nodiscard]] hi_inline generator<std::filesystem::path> theme_dirs() noexcept
{
    for (auto const& path : resource_dirs()) {
        co_yield path;
    }
}

} // namespace v1
} // namespace hi::inline v1
