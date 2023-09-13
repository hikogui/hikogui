// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../coroutine/module.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <filesystem>
#include <ranges>
#include <fstream>
#include <string>

/** @file path/path_location.hpp functions to locate files and directories.
 * @ingroup path
 */

hi_export_module(hikogui.path.path_location : intf);

namespace hi { inline namespace v1 {

/** File and Directory locations.
 * @ingroup file
 */
hi_export enum class path_location {
    /** The location of application resources.
     * 
     * The following directories are returned:
     * ```
     * if application is installed
     *   - application_install_dir / "resources"
     *   - data_dir / "resources"
     * else
     *   - application_build_dir / "resources"
     *   - application_source_dir / "resources"
     *   if HikoGUI is installed
     *     - hikogui_install_dir / "resources"
     *   else
     *     - hikogui_build_dir / "resources"
     *     - hikogui_source_dir / "resources"
     * ```
     */
    resource_dirs,

    /** A single file where the current running executable is located.
     */
    executable_file,

    /** The directory where the executable is located.
     */
    executable_dir,

    /** A single file where the current running HikoGUI shared library is located.
     * If HikoGUI is build as a static library then this will return the current executable instead.
     */
    library_file,

    /** The single directory where the HikoGUI shared library is located.
     */
    library_dir,

    /** The single directory where HikoGUI was installed when the application was build.
     */
    library_install_dir,

    /** The single directory where HikoGUI was build.
     */
    library_build_dir,

    /** The single directory where the HikoGUI source directory was when HikoGUI was build.
     */
    library_source_dir,

    /** The single directory where the data for the application is stored for the current user account.
     */
    data_dir,

    /** The single directory where to store the log files.
     */
    log_dir,

    /** A single file where to store or load the application preferences file for the current user account.
     */
    preferences_file,

    /** The directories where the system fonts are stored.
     */
    system_font_dirs,

    /** The directories where the fonts for the system and resource fonts are located.
     */
    font_dirs,

    /** The directories where the themes are located.
     */
    theme_dirs,
};

/** Get a set of paths.
 * @ingroup file
 *
 * @param location The location.
 * @return A list of paths belonging to the location.
 */
hi_export [[nodiscard]] generator<std::filesystem::path> get_paths(path_location location);

/** Find a path.
 * @ingroup file
 *
 * @param location The location to search for filesystem-object.
 * @param ref A relative path to the filesystem-object.
 * @return The the first full path to the filesystem-object found in the location. Or empty if the path is not found.
 */
hi_export [[nodiscard]] inline std::optional<std::filesystem::path>
find_path(path_location location, std::filesystem::path const& ref) noexcept
{
    if (ref.is_absolute()) {
        if (std::filesystem::exists(ref)) {
            return ref;
        } else {
            return {};
        }
    } else {
        for (hilet& base : get_paths(location)) {
            auto path = base / ref;
            if (std::filesystem::exists(path)) {
                return path;
            }
        }
        return {};
    }
}

/** Get the single and only path.
 * @ingroup file
 *
 * @param location The location.
 * @return The path.
 * @throw When there is not exactly one path.
 */
hi_export [[nodiscard]] inline std::filesystem::path get_path(path_location location)
{
    auto range = get_paths(location);
    auto it = std::ranges::begin(range);
    hilet last = std::ranges::end(range);

    if (it == last) {
        throw url_error("No path found.");
    }

    auto path = *it++;

    hi_assert(it == last, "More than one path found.");
    return path;
}

/** Get a line from an input string, upto a maximum size.
 * 
 * @post The input stream is read upto and including the line termination.
 * @param in The input stream.
 * @param max_size The maximum number of characters to read.
 * @return A string containing a line of characters, excluding the line termination.
 */
hi_export template<typename CharT, typename Traits = std::char_traits<CharT>>
[[nodiscard]] inline std::basic_string<CharT, Traits> getline(std::basic_istream<CharT, Traits>& in, size_t max_size) noexcept
{
    auto r = std::basic_string<CharT, Traits>{};

    while (r.size() < max_size) {
        auto c = in.get();
        if (c == Traits::eof()) {
            break;

        } else if (c == '\r') {
            c = in.get();
            if (c != '\n') {
                in.unget();
            }
            break;

        } else if (c == '\n') {
            break;
        }

        r += Traits::to_char_type(c);
    }

    return r;
}

/** Get the application's source path if the application is run from the build directory.
 *
 * @return The path to the source directory, or empty when the application is not
 *         in the build directory.
 */
hi_export [[nodiscard]] inline std::filesystem::path application_source_path() noexcept
{
    using namespace std::literals;

    // If the cmake_install.cmake file exists then the executable is located in a build directory.
    hilet cmake_install_path = get_path(path_location::executable_dir) / "cmake_install.cmake";

    if (std::filesystem::exists(cmake_install_path)) {
        auto line = std::string{};
        try {
            auto fd = std::ifstream{cmake_install_path.string()};
            line = getline(fd, 512);
            fd.close();

        } catch (std::exception const& e) {
            hi_log_error("Could not open file {}: {}", cmake_install_path.string(), e.what());
            return {};
        }

        hilet cmake_install_start = "# Install script for directory: "s;
        if (not line.starts_with(cmake_install_start)) {
            hi_log_error("File {} did not start with '{}'", cmake_install_path.string(), cmake_install_start);
            return {};
        }

        auto source_dir = std::filesystem::path{line.substr(cmake_install_start.size())};
        if (not std::filesystem::exists(source_dir)) {
            hi_log_error("Source directory {} does not exist", source_dir.string());
            return {};
        }

        return source_dir;

    } else {
        return {};
    }
}

}} // namespace hi::v1
