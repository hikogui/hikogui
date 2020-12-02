// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "theme_mode.hpp"
#include "Theme.hpp"
#include <limits>
#include <vector>
#include <new>


namespace tt {

/** ThemeBook keeps track of multiple themes.
 * The ThemeBook is instantiated during application startup
 * 
 */
class ThemeBook {
    std::vector<std::unique_ptr<Theme>> themes;
    std::string _themeName;
    theme_mode _themeMode;

    static inline char const *defaultThemeName = "TTauri";

public:
    ThemeBook(std::vector<URL> const &theme_directories) noexcept;

    [[nodiscard]] std::vector<std::string> themeNames() const noexcept;

    [[nodiscard]] theme_mode themeMode() const noexcept;

    void settheme_mode(theme_mode themeMode) noexcept;

    [[nodiscard]] std::string themeName() const noexcept;

    void setThemeName(std::string const &themeName) noexcept;

    void updateTheme() noexcept;
};

}
