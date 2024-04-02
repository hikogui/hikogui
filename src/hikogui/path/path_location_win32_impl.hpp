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

[[nodiscard]] hi_inline std::expected<std::filesystem::path, std::error_code> executable_file() noexcept
{
    static auto r = []() -> std::expected<std::filesystem::path, std::error_code> {
        if (auto path = win32_GetModuleFileName()) {
            return *path;
        } else {
            return std::unexpected{std::error_code{path.error()}};
        }
    }();

    return r;
}

[[nodiscard]] hi_inline std::expected<std::filesystem::path, std::error_code> data_dir() noexcept
{
    static auto r = []() -> std::expected<std::filesystem::path, std::error_code> {
        // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\"
        // FOLDERID_LocalAppData has the default path: %LOCALAPPDATA% (%USERPROFILE%\AppData\Local)
        if (auto path = win32_SHGetKnownFolderPath(FOLDERID_LocalAppData)) {
            return *path / get_application_vendor() / get_application_name() / "";
        } else {
            return std::unexpected{std::error_code{path.error()}};
        }
    }();

    return r;
}

[[nodiscard]] hi_inline std::expected<std::filesystem::path, std::error_code> log_dir() noexcept
{
    // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\Log\"
    if (auto path = data_dir()) {
        return *path / "Log" / "";
    } else {
        return std::unexpected{path.error()};
    }
}

[[nodiscard]] hi_inline generator<std::filesystem::path> resource_dirs() noexcept
{
    // Always look at the resource directory where the executable is located.
    if (auto path = executable_dir()) {
        co_yield *path / "resources" / "";
    }

    // Also look in the data directory of the application.
    if (auto path = data_dir()) {
        co_yield *path / "resources" / "";
    }

    // If the executable of the application is located in the build directory,
    // then check the source directories for resources.
    if (auto source_dir_ = source_dir()) {
        co_yield *source_dir_ / "resources" / "";

        // Check the in-tree HikoGUI-library build directory.
        if (auto path = library_cmake_build_dir(); not path.empty()) {
            co_yield path / "resources" / "";
        }

        // Check the in-tree HikoGUI-library source directory.
        if (auto path = library_cmake_source_dir(); not path.empty()) {
            co_yield path / "resources" / "";
        }

        // Check the HikoGUI source directory.
        co_yield library_source_dir() / "resources" / "";

        // Check the HikoGUI install directory.
        co_yield library_source_dir() / "share" / "hikogui" / "resources" / "";
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
