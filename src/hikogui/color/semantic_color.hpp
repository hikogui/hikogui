// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../enum_metadata.hpp"
#include <array>

namespace hi::inline v1 {

enum class semantic_color : unsigned char {
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
    primary_cursor,
    secondary_cursor,
};

// clang-format off
constexpr auto semantic_color_metadata = enum_metadata{
    semantic_color::blue, "blue",
    semantic_color::green, "green",
    semantic_color::indigo, "indigo",
    semantic_color::orange, "orange",
    semantic_color::pink, "pink",
    semantic_color::purple, "purple",
    semantic_color::red, "red",
    semantic_color::teal, "teal",
    semantic_color::yellow, "yellow",
    semantic_color::gray, "gray",
    semantic_color::gray2, "gray2",
    semantic_color::gray3, "gray3",
    semantic_color::gray4, "gray4",
    semantic_color::gray5, "gray5",
    semantic_color::gray6, "gray6",
    semantic_color::foreground, "foreground",
    semantic_color::border, "border",
    semantic_color::fill, "fill",
    semantic_color::accent, "accent",
    semantic_color::text_select, "text-select",
    semantic_color::primary_cursor, "primary-cursor",
    semantic_color::secondary_cursor, "secondary-cursor",
};

// clang-format on

[[nodiscard]] inline std::string_view to_string(semantic_color rhs) noexcept
{
    return semantic_color_metadata[rhs];
}


[[nodiscard]] inline semantic_color semantic_color_from_string(std::string_view str)
{
    return semantic_color_metadata[str];
}

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::semantic_color, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::semantic_color const &t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(hi::semantic_color_metadata[t], fc);
    }
};

