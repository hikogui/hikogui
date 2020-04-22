// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Theme.hpp"
#include <limits>
#include <array>
#include <new>


namespace TTauri::GUI {

/** ThemeBook keeps track of multiple themes.
 * The ThemeBook is instantiated during Application startup
 * 
 */
class ThemeBook {
    std::vector<std::unique_ptr<Theme>> themes;

public:
    ThemeBook(std::vector<URL> const &theme_directories);


};

inline ThemeBook *themeBook;

}
