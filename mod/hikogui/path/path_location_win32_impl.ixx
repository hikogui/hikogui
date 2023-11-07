// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"
#include "../win32_headers.hpp"


#include <filesystem>
#include <string>
#include <coroutine>

export module hikogui_path_path_location : impl;
import : intf;
import hikogui_char_maps; // XXX #616
import hikogui_coroutine;
import hikogui_metadata;
import hikogui_telemetry;
import hikogui_utility;

export namespace hi::inline v1 {

/** Convenience function for SHGetKnownFolderPath().
 *  Retrieves a full path of a known folder identified by the folder's KNOWNFOLDERID.
 *  See https://docs.microsoft.com/en-us/windows/win32/shell/knownfolderid#constants
 *
 * @param KNOWNFOLDERID folder_id.
 * @return The path of the folder.
 */
export [[nodiscard]] std::filesystem::path get_path_by_id(const KNOWNFOLDERID& folder_id) noexcept
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

export [[nodiscard]] std::filesystem::path get_module_path(HMODULE module_handle) noexcept
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
    hi_no_default("Could not get module path. It exceeds the buffer length of 32768 chars.");
}

export [[nodiscard]] std::filesystem::path executable_file() noexcept
{
    return get_module_path(nullptr);
}

export [[nodiscard]] std::filesystem::path data_dir() noexcept
{
    // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\"
    // FOLDERID_LocalAppData has the default path: %LOCALAPPDATA% (%USERPROFILE%\AppData\Local)
    hilet local_app_data = get_path_by_id(FOLDERID_LocalAppData);
    return local_app_data / get_application_vendor() / get_application_name() / "";
}

export [[nodiscard]] std::filesystem::path log_dir() noexcept
{
    // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\Log\"
    return data_dir() / "Log" / "";
}

export [[nodiscard]] std::filesystem::path preferences_file() noexcept
{
    // "%LOCALAPPDATA%\<Application Vendor>\<Application Name>\preferences.json"
    return data_dir() / "preferences.json";
}

export [[nodiscard]] generator<std::filesystem::path> resource_dirs() noexcept
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

export [[nodiscard]] generator<std::filesystem::path> system_font_dirs() noexcept
{
    co_yield get_path_by_id(FOLDERID_Fonts);
}

export [[nodiscard]] generator<std::filesystem::path> font_dirs() noexcept
{
    for (hilet& path : resource_dirs()) {
        co_yield path;
    }
    for (hilet& path : system_font_dirs()) {
        co_yield path;
    }
}

export [[nodiscard]] generator<std::filesystem::path> theme_dirs() noexcept
{
    for (hilet& path : resource_dirs()) {
        co_yield path;
    }
}

} // namespace hi::inline v1
