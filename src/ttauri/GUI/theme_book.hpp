// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "theme_mode.hpp"
#include "theme.hpp"
#include <limits>
#include <vector>
#include <new>


namespace tt {

/** theme_book keeps track of multiple themes.
 * The theme_book is instantiated during application startup
 * 
 */
class theme_book {
public:
    static inline std::unique_ptr<theme_book> global;

    theme_book(std::vector<URL> const &theme_directories) noexcept;

    [[nodiscard]] std::vector<std::string> theme_names() const noexcept;

    [[nodiscard]] tt::theme_mode current_theme_mode() const noexcept;

    void set_current_theme_mode(tt::theme_mode theme_mode) noexcept;

    [[nodiscard]] std::string current_theme_name() const noexcept;

    void set_current_theme_name(std::string const &themeName) noexcept;

private:
    std::vector<std::unique_ptr<theme>> themes;
    std::string _current_theme_name;
    tt::theme_mode _current_theme_mode;

    static inline char const *_default_theme_name = "TTauri";

    /** Find a theme matching the current name and mode.
     */
    void update_theme() noexcept;
};

}
