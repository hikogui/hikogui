// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "cmake_install.hpp"
#include "../utility/utility.hpp"
#include "../metadata/metadata.hpp"
#include "../macros.hpp"
#include <filesystem>
#include <ranges>
#include <fstream>
#include <string>
#include <ranges>
#include <expected>
#include <system_error>

hi_warning_push();
// C4702 unreachable code: False positive, but "not a bug" / "low priority".
// https://developercommunity.visualstudio.com/t/warning-c4702-for-range-based-for-loop/859129
// The ranged for-loop is translated into a normal for-loop. The iterator
// part of the normal for-loop is never executed due to the immediate return
// inside the for-loop.
hi_warning_ignore_msvc(4702);

/** @file path/path_location.hpp functions to locate files and directories.
 * @ingroup path
 */

hi_export_module(hikogui.path.path_location : intf);

hi_export namespace hi { inline namespace v1 {


template<typename Context>
concept path_range =
    std::ranges::input_range<Context> and
    std::convertible_to<std::ranges::range_value_t<std::remove_cvref_t<Context>>, std::filesystem::path> and
    not std::convertible_to<Context, std::filesystem::path>;

/** Find a path.
 * @ingroup path
 *
 * @param locations The locations to search for filesystem-object.
 * @param ref A relative path to the filesystem-object.
 * @return full paths to the filesystem-objects found in the location.
 */
template<path_range Locations>
[[nodiscard]] hi_inline generator<std::filesystem::path>
find_path(Locations &&locations, std::filesystem::path const& ref) noexcept
{
    if (ref.is_absolute()) {
        if (std::filesystem::exists(ref)) {
            co_yield ref;
        }

    } else {
        for (auto const& base : locations) {
            auto const path = base / ref;
            if (std::filesystem::exists(path)) {
                co_yield path;
            }
        }
    }
}

/** Find a path.
 * @ingroup path
 *
 * @param location The location to search for filesystem-object.
 * @param ref A relative path to the filesystem-object.
 * @return full paths to the filesystem-objects found in the location.
 */
[[nodiscard]] hi_inline generator<std::filesystem::path>
find_path(std::filesystem::path const& location, std::filesystem::path const& ref) noexcept
{
    if (ref.is_absolute()) {
        if (std::filesystem::exists(ref)) {
            co_yield ref;
        }

    } else {
        auto const path = location / ref;
        if (std::filesystem::exists(path)) {
            co_yield path;
        }
    }
}

/** Get a path.
 * @ingroup path
 *
 * @param locations The locations to search for filesystem-object.
 * @param ref A relative path to the filesystem-object.
 * @return The full path to the first filesystem-object found in the location.
 * @throws io_error When a path is not found.
 */
template<path_range Locations>
[[nodiscard]] hi_inline std::filesystem::path get_path(Locations&& locations, std::filesystem::path const& ref)
{
    for (auto const &path: find_path(locations, ref)) {
        return path;
    }

    throw io_error(std::format("Could not find '{}' in search-path: {}", ref.string(), to_string(locations)));
}

/** Get a path.
 * @ingroup path
 *
 * @param location The locations to search for filesystem-object.
 * @param ref A relative path to the filesystem-object.
 * @return The full path to the first filesystem-object found in the location.
 * @throws io_error When a path is not found.
 */
[[nodiscard]] hi_inline std::filesystem::path get_path(std::filesystem::path const& location, std::filesystem::path const& ref)
{
    for (auto const &path: find_path(location, ref)) {
        return path;
    }

    throw io_error(std::format("Could not find '{}' in: {}", ref.string(), location.string()));
}

/** Get a path.
 * @ingroup path
 *
 * @param location The locations to search for filesystem-object.
 * @param ref A relative path to the filesystem-object.
 * @return The full path to the first filesystem-object found in the location.
 * @throws io_error When a path is not found.
 */
[[nodiscard]] hi_inline std::filesystem::path get_path(std::expected<std::filesystem::path, std::error_code> const& location, std::filesystem::path const& ref)
{
    if (not location) {
        throw io_error(std::format("Could not find '{}' because of an error at the location: {}", ref.string(), location.error().message()));
    }

    for (auto const &path: find_path(*location, ref)) {
        return path;
    }

    throw io_error(std::format("Could not find '{}' in: {}", ref.string(), location->string()));
}

/** Get a string representation of a search-path.
 * 
 * @param locations A range of std::filesystem::path elements.
 * @return A string of semicolon ';' separated paths.
 */
template<path_range Locations>
[[nodiscard]] hi_inline std::string to_string(Locations &&locations) noexcept
{
    auto r = std::string{};
    for (auto const& path: locations) {
        if (not r.empty()) {
            r += ";";
        }
        r += path.string();
    }
    return r;
}

/** Get the full path to this executable.
 * @ingroup path
 */
[[nodiscard]] std::expected<std::filesystem::path, std::error_code> executable_file() noexcept;

/** Get the full path to the directory when this executable is located.
 * @ingroup path
 */
[[nodiscard]] hi_inline std::expected<std::filesystem::path, std::error_code> executable_dir() noexcept
{
    auto path = executable_file();
    if (path) {
        path->remove_filename();
    }
    return path;
}

/** Get the full path to the directory where the application should store its data.
 * @ingroup path
 */
[[nodiscard]] std::expected<std::filesystem::path, std::error_code> data_dir() noexcept;

/** Get the full path to the directory where the application should store its log files.
 * @ingroup path
 */
[[nodiscard]] std::expected<std::filesystem::path, std::error_code> log_dir() noexcept;

/** The directories to search for resource files.
 * @ingroup path
 */
[[nodiscard]] hi_inline generator<std::filesystem::path> resource_dirs() noexcept;

/** The directories to search for system font files.
 * @ingroup path
 */
[[nodiscard]] hi_inline generator<std::filesystem::path> system_font_dirs() noexcept;

/** The directories to search for font files of both the application and system.
 * @ingroup path
 */
[[nodiscard]] hi_inline generator<std::filesystem::path> font_files() noexcept;

/** The directories to search for theme files of the application.
 * @ingroup path
 */
[[nodiscard]] hi_inline generator<std::filesystem::path> theme_files() noexcept;

/** Get the full path to source code of this executable.
 *
 * @ingroup path
 * @return The path to directory of the source code.
 * @retval std::nullopt The executable is not located in its build directory.
 */
[[nodiscard]] hi_inline std::optional<std::filesystem::path> source_dir() noexcept
{
    auto const executable_dir_ = executable_dir();
    if (not executable_dir_) {
        return std::nullopt;
    }

    // If the cmake_install.cmake file exists then the executable is located in a build directory.
    if (auto tmp = parse_cmake_install(*executable_dir_ / "cmake_install.cmake")) {
        return tmp->source_dir;
    }

    // When using a cmake multi-config generator The executable lives in the
    // ./Debug/, ./Release/ or ./RelWithDebInfo/ directory.
    // So, the cmake_install.cmake file is located one directory up.
    if (auto tmp = parse_cmake_install(*executable_dir_ / ".." / "cmake_install.cmake")) {
        return tmp->source_dir;
    }

    return std::nullopt;
}

[[nodiscard]] hi_inline std::filesystem::path library_source_dir() noexcept
{
    auto path = std::filesystem::path{__FILE__};
    path.replace_filename("../../..");
    return path.lexically_normal();
}

[[nodiscard]] hi_inline std::filesystem::path library_test_data_dir() noexcept
{
    return hi::library_source_dir() / "tests" / "data";
}

}} // namespace hi::v1

hi_warning_pop();
