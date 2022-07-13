

#pragma once

#include "../required.hpp"
#include "../cast.hpp"
#include "../enum_metadata.hpp"
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
    none = 0,
    regular = 1 << to_underlying(text_phrasing::regular),
    emphesis = 1 << to_underlying(text_phrasing::emphesis),
    strong = 1 << to_underlying(text_phrasing::strong),
    code = 1 << to_underlying(text_phrasing::code),
    abbreviation = 1 << to_underlying(text_phrasing::abbreviation),
    bold = 1 << to_underlying(text_phrasing::bold),
    italic = 1 << to_underlying(text_phrasing::italic),
    citation = 1 << to_underlying(text_phrasing::citation),
    keyboard = 1 << to_underlying(text_phrasing::keyboard),
    mark = 1 << to_underlying(text_phrasing::highlight),
    math = 1 << to_underlying(text_phrasing::math),
    example = 1 << to_underlying(text_phrasing::example),
    unarticulated = 1 << to_underlying(text_phrasing::unarticulated),

    all = regular | emphesis | strong | code | abbreviation | bold | italic | citation | keyboard | highlight | math | example |
        unarticulated
};

[[nodiscard]] constexpr text_phrasing_mask operator&(text_phrasing_mask const& lhs, text_phrasing_mask const& rhs) noexcept
{
    return static_cast<text_phrasing_mask>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr text_phrasing_mask operator|(text_phrasing_mask const& lhs, text_phrasing_mask const& rhs) noexcept
{
    return static_cast<text_phrasing_mask>(to_underlying(lhs) | to_underlying(rhs));
}

constexpr text_phrasing_mask &operator|=(text_phrasing_mask &lhs, text_phrasing_mask const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

constexpr text_phrasing_mask &operator&=(text_phrasing_mask &lhs, text_phrasing_mask const& rhs) noexcept
{
    return lhs = lhs & rhs;
}

[[nodiscard]] constexpr text_phrasing_mask to_text_phrasing_mask(text_phrasing const& rhs) noexcept
{
    hi_axiom(to_underlying(rhs) < sizeof(text_phrasing_mask) * CHAR_BIT);
    return static_cast<text_phrasing_mask>(1 << to_underlying(rhs));
}

[[nodiscard]] constexpr text_phrasing_mask to_text_phrasing_mask(char const &rhs) noexcept
{
    // clang-format off
    switch (rhs) {
    case 'r': return text_phrasing_mask::regular;
    case 'e': return text_phrasing_mask::emphesis;
    case 's': return text_phrasing_mask::strong;
    case 'c': return text_phrasing_mask::code;
    case 'a': return text_phrasing_mask::abbreviation;
    case 'b': return text_phrasing_mask::bold;
    case 'i': return text_phrasing_mask::italic;
    case 'c': return text_phrasing_mask::citation;
    case 'k': return text_phrasing_mask::keyboard;
    case 'h': return text_phrasing_mask::highlight;
    case 'm': return text_phrasing_mask::mark;
    case 'x': return text_phrasing_mask::example;
    case 'u': return text_phrasing_mask::unarticulated;
    default: hi_no_default();
    }
    // clang-format on
}

[[nodiscard]] constexpr text_phrasing_mask to_text_phrasing_mask(std::string_view rhs) noexcept
{
    auto r = text_phrasing_mask::none;
    for (hilet c: rhs) {
        r |= to_text_phrasing_mask(c);
    }
    return r;
}

[[nodiscard]] constexpr std::string to_string(text_phrasing_mask const &rhs) noexcept
{
    auto r = std::string{};
    r.reserve(std::popcount(rhs));

    // clang-format off
    if (to_bool(rhs & text_phrasing_mask::regular))       { r += 'r'; }
    if (to_bool(rhs & text_phrasing_mask::emphesis))      { r += 'e'; }
    if (to_bool(rhs & text_phrasing_mask::strong))        { r += 's'; }
    if (to_bool(rhs & text_phrasing_mask::code))          { r += 'c'; }
    if (to_bool(rhs & text_phrasing_mask::abbreviation))  { r += 'a'; }
    if (to_bool(rhs & text_phrasing_mask::bold))          { r += 'b'; }
    if (to_bool(rhs & text_phrasing_mask::italic))        { r += 'i'; }
    if (to_bool(rhs & text_phrasing_mask::citation))      { r += 'q'; }
    if (to_bool(rhs & text_phrasing_mask::keyboard))      { r += 'k'; }
    if (to_bool(rhs & text_phrasing_mask::highlight))     { r += 'h'; }
    if (to_bool(rhs & text_phrasing_mask::math))          { r += 'm'; }
    if (to_bool(rhs & text_phrasing_mask::example))       { r += 'x'; }
    if (to_bool(rhs & text_phrasing_mask::unarticulated)) { r += 'u'; }
    // clang-format on

    return r;
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
