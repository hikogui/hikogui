// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme_book.hpp"
#include "../trace.hpp"

namespace tt {

theme_book::theme_book(std::vector<URL> const &theme_directories) noexcept :
    themes(), _current_theme_name(), _current_theme_mode(theme_mode::light)
{
    for (ttlet &theme_directory: theme_directories) {
        ttlet theme_directory_glob = theme_directory / "**" / "*.theme.json";
        for (ttlet &theme_url: theme_directory_glob.urlsByScanningWithGlobPattern()) {
            auto t = trace<"theme_scan">{};

            try {
                themes.push_back(std::make_unique<theme>(theme_url));
            } catch (std::exception const &e) {
                tt_log_error("Failed parsing theme at {}. \"{}\"", theme_url, e.what());
            }
        }
    }

    if (std::ssize(themes) == 0) {
        tt_log_fatal("Could not parse any themes.");
    }

    update_theme();
}

[[nodiscard]] std::vector<std::string> theme_book::theme_names() const noexcept {
    std::vector<std::string> names;

    for (ttlet &t: themes) {
        names.push_back(t->name);
    }

    std::sort(names.begin(), names.end());
    ttlet new_end = std::unique(names.begin(), names.end());
    names.erase(new_end, names.cend());
    return names;
}

[[nodiscard]] tt::theme_mode theme_book::current_theme_mode() const noexcept {
    return _current_theme_mode;
}

void theme_book::set_current_theme_mode(tt::theme_mode theme_mode) noexcept {
    _current_theme_mode = theme_mode;
    update_theme();
}

[[nodiscard]] std::string theme_book::current_theme_name() const noexcept {
    return _current_theme_name;
}

void theme_book::set_current_theme_name(std::string const &theme_name) noexcept
{
    _current_theme_name = theme_name;
    update_theme();
}

void theme_book::update_theme() noexcept
{
    theme *default_theme = nullptr;
    theme *default_theme_and_mode = nullptr;
    theme *matching_theme = nullptr;
    theme *matching_theme_and_mode = nullptr;

    for (auto &t: themes) {
        if (t->name == _current_theme_name && t->mode == _current_theme_mode) {
            matching_theme_and_mode = t.get();
        } else if (t->name == _current_theme_name) {
            matching_theme = t.get();
        } else if (t->name == _default_theme_name && t->mode == _current_theme_mode) {
            default_theme_and_mode = t.get();
        } else if (t->name == _default_theme_name) {
            default_theme = t.get();
        }
    }

    if (matching_theme_and_mode) {
        theme::global = matching_theme_and_mode;
    } else if (matching_theme) {
        theme::global = matching_theme;
    } else if (default_theme_and_mode) {
        theme::global = default_theme_and_mode;
    } else if (default_theme) {
        theme::global = default_theme;
    } else if (std::ssize(themes) > 0) {
        theme::global = themes[0].get();
    } else {
        tt_no_default();
    }

    tt_log_info("theme changed to {}, operating system mode {}", to_string(*theme::global), _current_theme_mode);
}


}
