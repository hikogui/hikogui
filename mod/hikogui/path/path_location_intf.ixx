// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <filesystem>
#include <ranges>
#include <fstream>
#include <string>
#include <ranges>

/** @file path/path_location.hpp functions to locate files and directories.
 * @ingroup path
 */

export module hikogui_path_path_location : intf;
import hikogui_coroutine;
import hikogui_metadata;
import hikogui_utility;

export namespace hi { inline namespace v1 {


export template<typename Context>
concept path_range =
    std::ranges::input_range<Context> and
    std::convertible_to<std::ranges::range_value_t<std::remove_cvref_t<Context>>, std::filesystem::path> and
    not std::convertible_to<Context, std::filesystem::path>;

/** Find a path.
 * @ingroup path
 *
 * @param locations The locations to search for filesystem-object.
 * @param ref A relative path to the filesystem-object.
 * @return The the first full path to the filesystem-object found in the location. Or empty if the path is not found.
 */
export template<path_range Locations>
[[nodiscard]] std::optional<std::filesystem::path>
find_path(Locations &&locations, std::filesystem::path const& ref) noexcept
{
    if (ref.is_absolute()) {
        if (std::filesystem::exists(ref)) {
            return ref;
        } else {
            return {};
        }
    } else {
        for (hilet& base : locations) {
            auto path = base / ref;
            if (std::filesystem::exists(path)) {
                return path;
            }
        }
        return {};
    }
}

/** Get the full path to this executable.
 * @ingroup path
 */
export [[nodiscard]] std::filesystem::path executable_file() noexcept;

/** Get the full path to the directory when this executable is located.
 * @ingroup path
 */
export [[nodiscard]] std::filesystem::path executable_dir() noexcept
{
    auto tmp = executable_file();
    tmp.remove_filename();
    return tmp;
}

/** Get the full path to the directory where the application should store its data.
 * @ingroup path
 */
export [[nodiscard]] std::filesystem::path data_dir() noexcept;

/** Get the full path to the directory where the application should store its log files.
 * @ingroup path
 */
export [[nodiscard]] std::filesystem::path log_dir() noexcept;

/** Get the full path to application preferences file.
 * @ingroup path
 */
export [[nodiscard]] std::filesystem::path preferences_file() noexcept;

/** The directories to search for resource files.
 * @ingroup path
 */
export [[nodiscard]] generator<std::filesystem::path> resource_dirs() noexcept;

/** The directories to search for system font files.
 * @ingroup path
 */
export [[nodiscard]] generator<std::filesystem::path> system_font_dirs() noexcept;

/** The directories to search for font files of both the application and system.
 * @ingroup path
 */
export [[nodiscard]] generator<std::filesystem::path> font_files() noexcept;

/** The directories to search for theme files of the application.
 * @ingroup path
 */
export [[nodiscard]] generator<std::filesystem::path> theme_files() noexcept;

/** Get the full path to source code of this executable.
 *
 * @ingroup path
 * @return The path to directory of the source code.
 * @retval std::nullopt The executable is not located in its build directory.
 */
export [[nodiscard]] std::optional<std::filesystem::path> source_dir() noexcept
{
    using namespace std::literals;

    // If the cmake_install.cmake file exists then the executable is located in a build directory.
    hilet cmake_install_path = executable_dir() / "cmake_install.cmake";

    if (std::filesystem::exists(cmake_install_path)) {
        auto line = std::string{};
        try {
            auto fd = std::ifstream{cmake_install_path.string()};
            line = getline(fd, 512);
            fd.close();

        } catch (...) {
            return std::nullopt;
        }

        hilet cmake_install_start = "# Install script for directory: "s;
        if (not line.starts_with(cmake_install_start)) {
            return std::nullopt;
        }

        auto source_dir = std::filesystem::path{line.substr(cmake_install_start.size())};
        if (not std::filesystem::exists(source_dir)) {
            return std::nullopt;
        }

        return source_dir;

    } else {
        return std::nullopt;
    }
}

/** The full path where HikoGUI is installed during compilation of the application.
 * 
 * @ingroup path
 * @return The full path to the install path of HikoGUI.
 * @retval std::nullopt The HikoGUI library is not installed and is located in its build dir.
 */
[[nodiscard]] std::optional<std::filesystem::path> library_install_dir() noexcept
{
    // path is:
    //  - /install_dir/include/hikogui/path/path_location_impl.hpp
    //  - /build_dir/src/hikogui/path/path_location_impl.hpp
    // becomes:
    //  - /install_dir/
    //  - /build_dir/
    auto path = std::filesystem::path{__FILE__};
    path.replace_filename("../../..");
    auto install_path = std::filesystem::canonical(path);
    
    if (install_path != library_source_dir()) {
        return install_path;
    } else {
        return std::nullopt;
    }
}

}} // namespace hi::v1
