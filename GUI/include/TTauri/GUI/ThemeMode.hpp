// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <string>
#include <ostream>

namespace TTauri::GUI {

enum class ThemeMode {
    Light,
    Dark,
    LightAccessable,
    DarkAccessable
};

[[nodiscard]] constexpr char const *to_const_string(ThemeMode rhs) noexcept {
    switch (rhs) {
    case ThemeMode::Light: return "Light";
    case ThemeMode::Dark: return "Dark";
    case ThemeMode::LightAccessable: return "LightAccessable";
    case ThemeMode::DarkAccessable: return "DarkAccessable";
    default: no_default;
    }
}

[[nodiscard]] inline std::string to_string(ThemeMode rhs) noexcept {
    return to_const_string(rhs);
}

inline std::ostream &operator<<(std::ostream &lhs, ThemeMode rhs) {
    return lhs << to_const_string(rhs);
}


ThemeMode readOSThemeMode() noexcept;

/** The current theme mode.
 */
inline ThemeMode themeMode = ThemeMode::Light;

}