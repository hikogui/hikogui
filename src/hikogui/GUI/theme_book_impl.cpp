// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "theme_book.hpp"
#include "../font/module.hpp"
#include "../file/glob.hpp"
#include "../utility/module.hpp"
#include "../trace.hpp"
#include "../log.hpp"

namespace hi::inline v1 {

theme_book::~theme_book() {}

theme_book::theme_book(hi::font_book const &font_book, std::vector<std::filesystem::path> const &theme_directories) noexcept : themes()
{
    for (hilet &theme_directory : theme_directories) {
        hilet theme_directory_glob = theme_directory / "**" / "*.theme.json";
        for (hilet &theme_path : glob(theme_directory_glob)) {
            auto t = trace<"theme_scan">{};

            try {
                themes.push_back(std::make_unique<theme>(font_book, theme_path));
            } catch (std::exception const &e) {
                hi_log_error("Failed parsing theme at {}. \"{}\"", theme_path.string(), e.what());
            }
        }
    }

    if (ssize(themes) == 0) {
        hi_log_fatal("Did not load any themes.");
    }
}

[[nodiscard]] std::vector<std::string> theme_book::theme_names() const noexcept
{
    std::vector<std::string> names;

    for (hilet &t : themes) {
        names.push_back(t->name);
    }

    std::sort(names.begin(), names.end());
    hilet new_end = std::unique(names.begin(), names.end());
    names.erase(new_end, names.cend());
    return names;
}

[[nodiscard]] theme const &theme_book::find(std::string name, theme_mode mode) const noexcept
{
    theme *default_theme = nullptr;
    theme *default_theme_and_mode = nullptr;
    theme *matching_theme = nullptr;
    theme *matching_theme_and_mode = nullptr;

    for (hilet &t : themes) {
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

} // namespace hi::inline v1
