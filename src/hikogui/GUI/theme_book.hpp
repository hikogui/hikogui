// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_mode.hpp"
#include "theme_file.hpp"
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
    static theme_book& global() noexcept;

    ~theme_book() = default;
    theme_book(theme_book const&) = delete;
    theme_book(theme_book&&) = delete;
    theme_book& operator=(theme_book const&) = delete;
    theme_book& operator=(theme_book&&) = delete;
    theme_book() = default;

    void register_theme_directory(std::filesystem::path const& path) noexcept
    {
        hilet theme_directory_glob = path / "**" / "*.theme.json";
        for (hilet& theme_path : glob(theme_directory_glob)) {
            auto t = trace<"theme:load">{};

            try {
                themes.push_back(std::make_unique<theme_file>(theme_path));
            } catch (std::exception const& e) {
                hi_log_error("Failed parsing theme at {}. \"{}\"", theme_path.string(), e.what());
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

        for (hilet& t : themes) {
            names.push_back(t->name);
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
    [[nodiscard]] theme_file const& find(std::string name, theme_mode mode) const noexcept
    {
        if (themes.empty()) {
            hi_log_fatal("No themes where loaded in the theme_book.");
        }

        theme_file *default_theme = nullptr;
        theme_file *default_theme_and_mode = nullptr;
        theme_file *matching_theme = nullptr;
        theme_file *matching_theme_and_mode = nullptr;

        for (hilet& t : themes) {
            if (t->name == name and t->mode == mode) {
                matching_theme_and_mode = t.get();
            } else if (t->name == name) {
                matching_theme = t.get();
            } else if (t->name == "default" and t->mode == mode) {
                default_theme_and_mode = t.get();
            } else if (t->name == "default") {
                default_theme = t.get();
            }
        }

        if (matching_theme_and_mode) {
            return *matching_theme_and_mode;
        } else if (matching_theme) {
            return *matching_theme;
        } else if (default_theme_and_mode) {
            return *default_theme_and_mode;
        } else if (default_theme) {
            return *default_theme;
        } else  {
            return *themes.front();
        }
    }

private:
    std::vector<std::unique_ptr<theme_file>> themes;
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
 * @return A theme most closely matching the requested theme.
 */
[[nodiscard]] inline theme_file const& find_theme(std::string name, theme_mode mode) noexcept
{
    return detail::theme_book::global().find(name, mode);
}

}} // namespace hi::v1
