// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#pragma once

#include "../utility/module.hpp"
#include <cstdint>

namespace hi::inline v1 {

enum class text_phrasing {
    regular,
    emphesis,
    strong,
    code,
    abbreviation,
    bold,
    italic,
    citation,
    keyboard, ///< Used in help messages to show which key to press.
    mark, ///< Yellow highlight
    math,
    example, ///< Used for displaying console output.
    unarticulated, ///< underlined
};

// clang-format off
constexpr auto text_phrasing_metadata = enum_metadata{
    text_phrasing::regular, "regular",
    text_phrasing::emphesis, "emphesis",
    text_phrasing::strong, "strong",
    text_phrasing::code, "code",
    text_phrasing::abbreviation, "abbreviation",
    text_phrasing::bold, "bold",
    text_phrasing::italic, "italic",
    text_phrasing::citation, "citation",
    text_phrasing::keyboard, "keyboard",
    text_phrasing::mark, "mark",
    text_phrasing::math, "math",
    text_phrasing::example, "example",
    text_phrasing::unarticulated, "unarticulated",
};
// clang-format on

enum class text_phrasing_mask : uint16_t {
    regular = 1 << to_underlying(text_phrasing::regular),
    emphesis = 1 << to_underlying(text_phrasing::emphesis),
    strong = 1 << to_underlying(text_phrasing::strong),
    code = 1 << to_underlying(text_phrasing::code),
    abbreviation = 1 << to_underlying(text_phrasing::abbreviation),
    bold = 1 << to_underlying(text_phrasing::bold),
    italic = 1 << to_underlying(text_phrasing::italic),
    citation = 1 << to_underlying(text_phrasing::citation),
    keyboard = 1 << to_underlying(text_phrasing::keyboard),
    mark = 1 << to_underlying(text_phrasing::mark),
    math = 1 << to_underlying(text_phrasing::math),
    example = 1 << to_underlying(text_phrasing::example),
    unarticulated = 1 << to_underlying(text_phrasing::unarticulated),

    all = regular | emphesis | strong | code | abbreviation | bold | italic | citation | keyboard | mark | math | example |
        unarticulated
};

[[nodiscard]] constexpr text_phrasing_mask to_text_phrasing_mask(text_phrasing const& rhs) noexcept
{
    hi_axiom(to_underlying(rhs) < sizeof(text_phrasing_mask) * CHAR_BIT);
    return static_cast<text_phrasing_mask>(1 << to_underlying(rhs));
}

[[nodiscard]] constexpr text_phrasing_mask operator&(text_phrasing_mask const& lhs, text_phrasing_mask const& rhs) noexcept
{
    return static_cast<text_phrasing_mask>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr text_phrasing_mask operator|(text_phrasing_mask const& lhs, text_phrasing_mask const& rhs) noexcept
{
    return static_cast<text_phrasing_mask>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr bool all(text_phrasing_mask const& rhs) noexcept
{
    return (rhs & text_phrasing_mask::all) == text_phrasing_mask::all;
}

[[nodiscard]] constexpr bool to_bool(text_phrasing_mask const& rhs) noexcept
{
    return to_bool(to_underlying(rhs));
}

} // namespace hi::inline v1
