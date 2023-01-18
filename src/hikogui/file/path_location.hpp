// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file file/path_location.hpp functions to locate files and directories.
 * @ingroup file
 */

#pragma once

#include "../generator.hpp"
#include "../utility/module.hpp"
#include <filesystem>
#include <ranges>

namespace hi { inline namespace v1 {

/** File and Directory locations.
 * @ingroup file
 */
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

/** Get a set of paths.
 * @ingroup file
 *
 * @param location The location.
 * @return A list of paths belonging to the location.
 */
[[nodiscard]] generator<std::filesystem::path> get_paths(path_location location);

/** Find a path.
 * @ingroup file
 *
 * @param location The location to search for filesystem-object.
 * @param ref A relative path to the filesystem-object.
 * @return The the first full path to the filesystem-object found in the location. Or empty if the path is not found.
 */
[[nodiscard]] inline std::optional<std::filesystem::path> find_path(path_location location, std::filesystem::path const &ref) noexcept
{
    if (ref.is_absolute()) {
        if (std::filesystem::exists(ref)) {
            return ref;
        } else {
            return {};
        }
    } else {
        for (hilet &base: get_paths(location)) {
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


}} // namespace hi::v1
