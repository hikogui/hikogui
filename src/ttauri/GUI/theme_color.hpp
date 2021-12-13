// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_text_style.hpp"
#include "../exception.hpp"

namespace tt::inline v1 {

enum class theme_color : unsigned char {
    blue,
    green,
    indigo,
    orange,
    pink,
    purple,
    red,
    teal,
    yellow,

    gray,
    gray2,
    gray3,
    gray4,
    gray5,
    gray6,

    foreground,
    border,
    fill,
    accent,
    text_select,
    cursor,
    incomplete_glyph,

    _size
};

constexpr std::size_t num_theme_colors = static_cast<std::size_t>(theme_color::_size);

[[nodiscard]] inline theme_color theme_color_from_string(std::string_view str)
{
    if (str == "blue") {
        return theme_color::blue;
    } else if (str == "green") {
        return theme_color::green;
    } else if (str == "indigo") {
        return theme_color::indigo;
    } else if (str == "orange") {
        return theme_color::orange;
    } else if (str == "pink") {
        return theme_color::pink;
    } else if (str == "purple") {
        return theme_color::purple;
    } else if (str == "red") {
        return theme_color::red;
    } else if (str == "teal") {
        return theme_color::teal;
    } else if (str == "yellow") {
        return theme_color::yellow;
    } else if (str == "gray") {
        return theme_color::gray;
    } else if (str == "gray2") {
        return theme_color::gray2;
    } else if (str == "gray3") {
        return theme_color::gray3;
    } else if (str == "gray4") {
        return theme_color::gray4;
    } else if (str == "gray5") {
        return theme_color::gray5;
    } else if (str == "gray6") {
        return theme_color::gray6;
    } else if (str == "foreground") {
        return theme_color::foreground;
    } else if (str == "border") {
        return theme_color::border;
    } else if (str == "fill") {
        return theme_color::fill;
    } else if (str == "accent") {
        return theme_color::accent;
    } else if (str == "text_select") {
        return theme_color::text_select;
    } else if (str == "cursor") {
        return theme_color::cursor;
    } else if (str == "incomplete_glyph") {
        return theme_color::incomplete_glyph;
    } else {
        throw parse_error("Unknown theme color '{}'", str);
    }
}

} // namespace tt::inline v1
