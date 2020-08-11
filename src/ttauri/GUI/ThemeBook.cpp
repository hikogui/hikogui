// Copyright 2020 Pokitec
// All rights reserved.

#include "ThemeBook.hpp"
#include "../trace.hpp"

namespace tt {

ThemeBook::ThemeBook(std::vector<URL> const &theme_directories) noexcept :
    themes(), _themeName(), _themeMode(ThemeMode::Light)
{
    for (ttlet &theme_directory: theme_directories) {
        ttlet theme_directory_glob = theme_directory / "**" / "*.theme.json";
        for (ttlet &theme_url: theme_directory_glob.urlsByScanningWithGlobPattern()) {
            struct theme_scan_tag {};
            auto t = trace<theme_scan_tag>{};

            try {
                themes.push_back(std::make_unique<Theme>(theme_url));
            } catch (error &) {
                LOG_ERROR("Failed parsing theme at {}", theme_url);
            }
        }
    }

    if (std::ssize(themes) == 0) {
        LOG_FATAL("Could not parse any themes.");
    }

    updateTheme();
}

[[nodiscard]] std::vector<std::string> ThemeBook::themeNames() const noexcept {
    std::vector<std::string> names;

    for (ttlet &t: themes) {
        names.push_back(t->name);
    }

    std::sort(names.begin(), names.end());
    ttlet new_end = std::unique(names.begin(), names.end());
    names.erase(new_end, names.cend());
    return names;
}

[[nodiscard]] ThemeMode ThemeBook::themeMode() const noexcept {
    return _themeMode;
}

void ThemeBook::setThemeMode(ThemeMode themeMode) noexcept {
    _themeMode = themeMode;
    updateTheme();
}

[[nodiscard]] std::string ThemeBook::themeName() const noexcept {
    return _themeName;
}

void ThemeBook::setThemeName(std::string const &themeName) noexcept {
    _themeName = themeName;
    updateTheme();
}

void ThemeBook::updateTheme() noexcept {
    Theme *defaultTheme = nullptr;
    Theme *defaultAndModeTheme = nullptr;
    Theme *matchingTheme = nullptr;
    Theme *matchingAndModeTheme = nullptr;

    for (auto &t: themes) {
        if (t->name == _themeName && t->mode == _themeMode) {
            matchingAndModeTheme = t.get();
        } else if (t->name == _themeName) {
            matchingTheme = t.get();
        } else if (t->name == defaultThemeName && t->mode == _themeMode) {
            defaultAndModeTheme = t.get();
        } else if (t->name == defaultThemeName) {
            defaultTheme = t.get();
        }
    }

    if (matchingAndModeTheme) {
        theme = matchingAndModeTheme;
    } else if (matchingTheme) {
        theme = matchingTheme;
    } else if (defaultAndModeTheme) {
        theme = defaultAndModeTheme;
    } else if (defaultTheme) {
        theme = defaultTheme;
    } else if (std::ssize(themes) > 0) {
        theme = themes[0].get();
    } else {
        tt_no_default;
    }

    LOG_INFO("Theme changed to {}, operating system mode {}", to_string(*theme), _themeMode);
}


}
