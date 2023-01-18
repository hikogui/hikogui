// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../utility/win32_headers.hpp"

#include "path_location.hpp"
#include "../log.hpp"
#include "../metadata.hpp"
#include "../defer.hpp"
#include <filesystem>
#include <string>

namespace hi::inline v1 {

/** Convenience function for SHGetKnownFolderPath().
 *  Retrieves a full path of a known folder identified by the folder's KNOWNFOLDERID.
 *  See https://docs.microsoft.com/en-us/windows/win32/shell/knownfolderid#constants
 *
 * @param KNOWNFOLDERID folder_id.
 * @return The path of the folder.
 */
[[nodiscard]] static std::filesystem::path get_path_by_id(const KNOWNFOLDERID& folder_id) noexcept
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

[[nodiscard]] static std::filesystem::path get_module_path(HMODULE module_handle) noexcept
{
    std::wstring module_path;
    auto buffer_size = MAX_PATH; // initial default value = 256

    // iterative buffer resizing to max value of 32768 (256*2^7)
    for (std::size_t i = 0; i < 7; ++i) {
        module_path.resize(buffer_size);
        auto chars = GetModuleFileNameW(module_handle, module_path.data(), buffer_size);
        if (chars < module_path.length()) {
            module_path.resize(chars);
            return std::filesystem::path{module_path};

        } else {
            buffer_size *= 2;
        }
    }
    hi_log_fatal("Could not get executable path. It exceeds the buffer length of 32768 chars.");
}

[[nodiscard]] static std::filesystem::path get_executable_path() noexcept
{
    return get_module_path(nullptr);
}

[[nodiscard]] hi_no_inline static std::filesystem::path get_library_path() noexcept
{
    HMODULE module_handle = nullptr;
    if (not GetModuleHandleEx(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, reinterpret_cast<LPCTSTR>(get_library_path), &module_handle)) {
        hi_log_fatal("Could not get a handle to the current module.");
    }
    hilet d = defer{[&] {
        FreeLibrary(module_handle);
    }};

    return get_module_path(module_handle);
}

[[nodiscard]] generator<std::filesystem::path> get_paths(path_location location)
{
    using enum path_location;

    switch (location) {
    case executable_file:
        co_yield get_executable_path();
        break;

    case executable_dir:
        co_yield get_path(executable_file).remove_filename();
        break;

    case library_file:
        co_yield get_library_path();
        break;

    case library_dir:
        co_yield get_path(library_file).remove_filename();
        break;

    case resource_dirs:
        {
            hilet executable_path = get_path(path_location::executable_dir);
            hilet library_path = get_path(path_location::library_dir);
            co_yield executable_path / "resources/";
            if (library_path != executable_path) {
                // XXX use the system-library resource path instead.
                co_yield library_path / "resources/";
            }
        }
        break;

    case data_dir:
        // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\"
        {
            // FOLDERID_LocalAppData has the default path: %LOCALAPPDATA% (%USERPROFILE%\AppData\Local)
            hilet local_app_data = get_path_by_id(FOLDERID_LocalAppData);
            if (metadata::application().vendor.empty()) {
                co_yield local_app_data / metadata::application().display_name / "";
            } else {
                co_yield local_app_data / metadata::application().vendor / metadata::application().display_name / "";
            }
        }
        break;

    case log_dir:
        // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\Log\"
        co_yield get_path(data_dir) / "Log/";
        break;

    case preferences_file:
        co_yield get_path(data_dir) / "preferences.json";
        break;

    case system_font_dirs:
        // FOLDERID_Fonts has the default path: %windir%\Fonts
        co_yield get_path_by_id(FOLDERID_Fonts);
        break;

    case font_dirs:
        // Sorted from global to local.
        for (hilet& path : get_paths(resource_dirs)) {
            co_yield path / "fonts" / "";
        }
        for (hilet& path : get_paths(system_font_dirs)) {
            co_yield path;
        }
        break;

    case theme_dirs:
        for (hilet& path : get_paths(resource_dirs)) {
            co_yield path / "themes" / "";
        }
        break;

    default:
        hi_no_default();
    }
}

} // namespace hi::inline v1
