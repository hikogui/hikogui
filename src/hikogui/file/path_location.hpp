// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file path_location.hpp function to locate files and directories.
 */

#pragma once

#include "../generator.hpp"
#include "../exception.hpp"
#include <filesystem>
#include <ranges>

namespace hi { inline namespace v1 {

enum class path_location {
    /** The location of application resources.
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

[[nodiscard]] generator<std::filesystem::path> get_paths(path_location location);

[[nodiscard]] inline std::filesystem::path get_path(path_location location)
{
    auto range = get_paths(location);
    auto it = std::ranges::begin(range);
    hilet last = std::ranges::end(range);

    if (it == last) {
        throw url_error("No path found.");
    }

    auto path = *it++;

    if (it != last) {
        throw url_error("More than one path found.");
    }

    return path;
}

[[nodiscard]] std::filesystem::path resource_directory() noexcept;
[[nodiscard]] std::filesystem::path executable_directory() noexcept;
[[nodiscard]] std::filesystem::path executable_path() noexcept;
[[nodiscard]] std::filesystem::path application_data_directory() noexcept;
[[nodiscard]] std::filesystem::path application_log_directory() noexcept;
[[nodiscard]] std::filesystem::path system_font_directory() noexcept;
[[nodiscard]] std::filesystem::path application_preferences_path() noexcept;
[[nodiscard]] std::vector<std::filesystem::path> font_directories() noexcept;

}} // namespace hi::v1
