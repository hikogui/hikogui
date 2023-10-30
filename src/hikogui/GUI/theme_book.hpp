// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme.hpp"
#include "../settings/settings.hpp"
#include "../macros.hpp"
#include <limits>
#include <vector>
#include <memory>
#include <filesystem>

hi_export_module(hikogui.GUI : theme_book);

hi_export namespace hi::inline v1 {

/** theme_book keeps track of multiple themes.
 *
 */
class theme_book {
public:
    /** The name of the selected theme.
     */
    observer<std::string> selected_theme = "default";

    ~theme_book() = default;
    theme_book(theme_book const&) = delete;
    theme_book(theme_book&&) = delete;
    theme_book& operator=(theme_book const&) = delete;
    theme_book& operator=(theme_book&&) = delete;
    theme_book() noexcept = default;

    static theme_book& global() noexcept;

    void register_directory(std::filesystem::path path) noexcept
    {
        _theme_directories.push_back(path);
    }

    void reload() noexcept
    {
        themes.clear();

        for (hilet& theme_directory : _theme_directories) {
            hilet theme_directory_glob = theme_directory / "**" / "*.theme.json";
            for (hilet& theme_path : glob(theme_directory_glob)) {
                auto t = trace<"theme_scan">{};

                try {
                    themes.push_back(std::make_unique<theme>(theme_path));
                } catch (std::exception const& e) {
                    hi_log_error("Failed parsing theme at {}. \"{}\"", theme_path.string(), e.what());
                }
            }
        }

        if (ssize(themes) == 0) {
            hi_log_fatal("Did not load any themes.");
        }
    }

    [[nodiscard]] std::vector<std::string> names() const noexcept
    {
        std::vector<std::string> names;

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
    [[nodiscard]] theme const& find(std::string_view name, theme_mode mode) const noexcept
    {
        theme *default_theme = nullptr;
        theme *default_theme_and_mode = nullptr;
        theme *matching_theme = nullptr;
        theme *matching_theme_and_mode = nullptr;

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
        } else if (ssize(themes) > 0) {
            return *themes[0].get();
        } else {
            hi_no_default();
        }
    }

private:
    std::vector<std::filesystem::path> _theme_directories;
    std::vector<std::unique_ptr<theme>> themes;
};

namespace detail {
hi_inline std::unique_ptr<theme_book> theme_book_global;
}

[[nodiscard]] hi_inline theme_book& theme_book::global() noexcept
{
    if (not detail::theme_book_global) {
        detail::theme_book_global = std::make_unique<theme_book>();
    }
    return *detail::theme_book_global;
}

hi_inline void register_theme_directory(std::filesystem::path const &path) noexcept
{
    return theme_book::global().register_directory(path);
}

hi_inline void reload_themes() noexcept
{
    return theme_book::global().reload();
}

template<std::ranges::range R>
hi_inline void register_theme_directories(R &&r) noexcept
{
    for (auto &path: r) {
        register_theme_directory(path);
    }
    reload_themes();
}

[[nodiscard]] hi_inline theme const& find_theme(std::string_view name, theme_mode mode) noexcept
{
    return theme_book::global().find(name, mode);
}

[[nodiscard]] hi_inline std::vector<std::string> theme_names() noexcept
{
    return theme_book::global().names();
}

[[nodiscard]] hi_inline theme const &get_selected_theme() noexcept
{
    return find_theme(*theme_book::global().selected_theme, os_settings::theme_mode());
}

} // namespace hi::inline v1
