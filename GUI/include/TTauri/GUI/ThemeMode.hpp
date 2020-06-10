// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <string>
#include <ostream>

namespace TTauri {

enum class ThemeMode {
    Light,
    Dark
};

[[nodiscard]] constexpr char const *to_const_string(ThemeMode rhs) noexcept {
    switch (rhs) {
    case ThemeMode::Light: return "Light";
    case ThemeMode::Dark: return "Dark";
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

float readOSToolbarHeight() noexcept;
float readOSToolbarDecorationButtonWidth() noexcept;

}