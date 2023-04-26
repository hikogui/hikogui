// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_mode.hpp"
#include "style_sheet.hpp"
#include "style_sheet_parser.hpp"
#include "../log.hpp"
#include "../trace.hpp"
#include "../file/glob.hpp"
#include <limits>
#include <vector>
#include <memory>
#include <filesystem>

namespace hi { inline namespace v1 {
namespace detail {

/** theme_book keeps track of multiple themes.
 *
 */
class theme_book {
public:
    [[nodiscard]] inline static theme_book& global() noexcept;

    ~theme_book() = default;
    theme_book(theme_book const&) = delete;
    theme_book(theme_book&&) = delete;
    theme_book& operator=(theme_book const&) = delete;
    theme_book& operator=(theme_book&&) = delete;
    theme_book() = default;

    void register_theme_directory(std::filesystem::path const& path) noexcept
    {
        theme_dirs.push_back(path);
        refresh();
    }

    /** Refresh the list of themes from the theme directories.
     */
    void refresh()
    {
        theme_files.clear();

        for (hilet& theme_dir : theme_dirs) {
            hilet theme_dir_glob = theme_dir / "**" / "*.css";
            for (hilet& path : glob(theme_dir_glob)) {
                try {
                    auto style_sheet = hi::parse_style_sheet(path);
                    hi_log_info("Found theme {}:{} at '{}'.", style_sheet.name, style_sheet.mode, path.generic_string());

                    theme_files.emplace_back(style_sheet.name, style_sheet.mode, path);

                } catch (std::exception const& e) {
                    hi_log_error("Unable to load theme from file '{}': {}", path.generic_string(), e.what());
                }
            }
        }
    }

    /** Get a list of theme names.
     *
     * This list of names is sorted and does not contain duplicates, ready
     * to be displayed to the user.
     *
     */
    [[nodiscard]] std::vector<std::string> names() const noexcept
    {
        auto names = std::vector<std::string>{};

        for (hilet& theme_file : theme_files) {
            names.push_back(theme_file.name);
        }

        std::sort(names.begin(), names.end());
        hilet new_end = std::unique(names.begin(), names.end());
        names.erase(new_end, names.cend());
        return names;
    }

    /** Find a theme matching the name and mode.
     *
     * @param name The name of the theme to select.
     * @param mode The mode of the theme to select.
     * @return A theme most closely matching the requested theme.
     */
    [[nodiscard]] std::optional<std::filesystem::path> find(std::string name, theme_mode mode) const noexcept
    {
        // First find the exact match.
        auto it = find_if(theme_files.begin(), theme_files.end(), [&](hilet& entry) {
            return entry.name == name and entry.mode == mode;
        });

        if (it != theme_files.end()) {
            return it->path;
        }

        // Next find the theme for different modes..
        it = find_if(theme_files.begin(), theme_files.end(), [&](hilet& entry) {
            return entry.name == name;
        });

        if (it != theme_files.end()) {
            return it->path;
        }

        return std::nullopt;
    }

private:
    struct theme_file_entry {
        std::string name;
        hi::theme_mode mode;
        std::filesystem::path path;
    };

    std::vector<std::filesystem::path> theme_dirs;
    std::vector<theme_file_entry> theme_files;
};

[[nodiscard]] inline theme_book& theme_book::global() noexcept
{
    static auto r = theme_book{};
    return r;
}

} // namespace detail

inline void register_theme_directory(std::filesystem::path const& path) noexcept
{
    return detail::theme_book::global().register_theme_directory(path);
}

/** Get a list of theme names.
 *
 * This list of names is sorted and does not contain duplicates, ready
 * to be displayed to the user.
 *
 */
[[nodiscard]] inline std::vector<std::string> theme_names() noexcept
{
    return detail::theme_book::global().names();
}

/** Find a theme matching the name and mode.
 *
 * @param name The name of the theme to select.
 * @param mode The mode of the theme to select.
 * @return true if the theme was loaded and activated successfully.
 */
inline bool load_theme(std::string name, theme_mode mode) noexcept
{
    if (hilet path = detail::theme_book::global().find(name, mode)) {
        try {
            parse_style_sheet(*path).activate();
            hi_log_info("Theme {} at '{}' activated successfully.", name, path->generic_string());
            return true;

        } catch (std::exception const& e) {
            hi_log_error("Unable to load theme {} from file '{}': {}", name, path->generic_string(), e.what());
            return false;
        }

    } else {
        hi_log_error("Unable to find a theme matching {}:{}", name, mode);
        return false;
    }
}

}} // namespace hi::v1
