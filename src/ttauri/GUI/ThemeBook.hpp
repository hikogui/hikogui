// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/GUI/ThemeMode.hpp"
#include "ttauri/GUI/Theme.hpp"
#include <limits>
#include <vector>
#include <new>


namespace tt {

/** ThemeBook keeps track of multiple themes.
 * The ThemeBook is instantiated during Application startup
 * 
 */
class ThemeBook {
    std::vector<std::unique_ptr<Theme>> themes;
    std::string _themeName;
    ThemeMode _themeMode;

    static inline char const *defaultThemeName = "TTauri";

public:
    ThemeBook(std::vector<URL> const &theme_directories) noexcept;

    [[nodiscard]] std::vector<std::string> themeNames() const noexcept;

    [[nodiscard]] ThemeMode themeMode() const noexcept;

    void setThemeMode(ThemeMode themeMode) noexcept;

    [[nodiscard]] std::string themeName() const noexcept;

    void setThemeName(std::string const &themeName) noexcept;

    void updateTheme() noexcept;
};

/** Global Theme Book.
 * set in globals.cpp
 */
inline ThemeBook *themeBook;

}
