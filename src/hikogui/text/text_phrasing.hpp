// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file text/text_phrasing.hpp The text_phrasing type.
 * @ingroup text
 */

#pragma once

#include "../utility/module.hpp"
#include <cstdint>
#include <optional>

namespace hi::inline v1 {

/** Text phrasing.
 * @ingroup text
 *
 * The underlying value must be between 0 through 15.
 */
enum class text_phrasing : uint8_t {
    /** Regular, normal text.
     */
    regular = 0,

    /** Emphesised text; spoken as if the text is special importance, significant or promonent.
     * Often formatted in italic.
     */
    emphesis = 1,

    /** Strong text; spoken louder, as if the text is not to be missed.
     * Often formatted in bold.
     */
    strong = 2,

    /** Text is a piece of programming-code; a variable name, a function name.
     * Often formatted in a constant-width font, with a greater weight and in a different color and possible
     * background block, than the surrounding text.
     */
    code = 3,

    /** An abbreviation.
     * Sometimes formatted with a double underline and hovering will show the expansion of the abbreviation.
     */
    abbreviation = 4,

    /** The text is quoted from somewhere.
     * Often formatted using a more italic / cursive font, with a lower weight.
     */
    quote = 5,

    /** Used in help text to show which key or button to press.
     * Often formatted with a background that looks raised up like a button.
     * With the text in inverted color.
     */
    keyboard = 6,

    /** The text is marked or highlighted as if being marked by a highlight pen.
     * Often formatted with a yellow background.
     */
    highlight = 7,

    /** Text formatted as math.
     * Often formatted using a special math font.
     */
    math = 8,

    /** Used in help text to show an example.
     * Often formatted using a non-proportional font with a low resultion bitmap-like style.
     */
    example = 9, ///< Used for displaying console output.

    /** Unarticulated.
     * Often formatted using an underlying line.
     */
    unarticulated = 10,

    /** Format a heading
     * Often in bold, larger font and on a line by itself.
     */
    title = 11,

    /** Format a "good" message
     * Often in bright green.
     */
    success = 12,

    /** Format a warning message
     * Often in bright yellow.
     */
    warning = 13,

    /** Format a "bad" message
     * Often in bright red.
     */
    error = 14,
};

// clang-format off
constexpr auto text_phrasing_metadata = enum_metadata{
    text_phrasing::regular, "regular",
    text_phrasing::emphesis, "emphesis",
    text_phrasing::strong, "strong",
    text_phrasing::code, "code",
    text_phrasing::abbreviation, "abbreviation",
    text_phrasing::quote, "quote",
    text_phrasing::keyboard, "keyboard",
    text_phrasing::highlight, "highlight",
    text_phrasing::math, "math",
    text_phrasing::example, "example",
    text_phrasing::unarticulated, "unarticulated",
    text_phrasing::title, "title",
    text_phrasing::success, "success",
    text_phrasing::warning, "warning",
    text_phrasing::error, "error",
};
// clang-format on
static_assert(text_phrasing_metadata.size() <= 15, "The mask of a text_phrasing is 16-bit");

// clang-format off
[[nodiscard]] constexpr std::optional<text_phrasing> to_text_phrasing(char c)
{
    switch (c) {
    case 'r': return text_phrasing::regular;
    case 'e': return text_phrasing::emphesis;
    case 's': return text_phrasing::strong;
    case 'c': return text_phrasing::code;
    case 'a': return text_phrasing::abbreviation;
    case 'q': return text_phrasing::quote;
    case 'k': return text_phrasing::keyboard;
    case 'h': return text_phrasing::highlight;
    case 'm': return text_phrasing::math;
    case 'x': return text_phrasing::example;
    case 'u': return text_phrasing::unarticulated;
    case 't': return text_phrasing::title;
    case 'S': return text_phrasing::success;
    case 'W': return text_phrasing::warning;
    case 'E': return text_phrasing::error;
    default:
        return std::nullopt;
    }
}
// clang-format on

enum class text_phrasing_mask : uint16_t {
    regular = 1 << to_underlying(text_phrasing::regular),
    emphesis = 1 << to_underlying(text_phrasing::emphesis),
    strong = 1 << to_underlying(text_phrasing::strong),
    code = 1 << to_underlying(text_phrasing::code),
    abbreviation = 1 << to_underlying(text_phrasing::abbreviation),
    quote = 1 << to_underlying(text_phrasing::quote),
    keyboard = 1 << to_underlying(text_phrasing::keyboard),
    highlight = 1 << to_underlying(text_phrasing::highlight),
    math = 1 << to_underlying(text_phrasing::math),
    example = 1 << to_underlying(text_phrasing::example),
    unarticulated = 1 << to_underlying(text_phrasing::unarticulated),
    title = 1 << to_underlying(text_phrasing::title),
    success = 1 << to_underlying(text_phrasing::success),
    warning = 1 << to_underlying(text_phrasing::warning),
    error = 1 << to_underlying(text_phrasing::error),

    all = regular | emphesis | strong | code | abbreviation | quote | keyboard | highlight | math | example | unarticulated |
        title | success | warning | error
};

[[nodiscard]] constexpr text_phrasing_mask operator&(text_phrasing_mask const& lhs, text_phrasing_mask const& rhs) noexcept
{
    return static_cast<text_phrasing_mask>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr text_phrasing_mask operator|(text_phrasing_mask const& lhs, text_phrasing_mask const& rhs) noexcept
{
    return static_cast<text_phrasing_mask>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr text_phrasing_mask to_text_phrasing_mask(text_phrasing const& rhs) noexcept
{
    hi_axiom(to_underlying(rhs) < sizeof(text_phrasing_mask) * CHAR_BIT);
    return static_cast<text_phrasing_mask>(1 << to_underlying(rhs));
}

[[nodiscard]] constexpr text_phrasing_mask to_text_phrasing_mask(std::string const& str)
{
    auto r = text_phrasing_mask{};

    for (hilet c : str) {
        if (c == '*') {
            r = text_phrasing_mask::all;
        } else if (hilet p = to_text_phrasing(c)) {
            r = r | to_text_phrasing_mask(*p);
        } else {
            throw parse_error(std::format("Unknown character '{}' in text-phrasing-mask", c));
        }
    }

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

/** Check if the text-phrasing is included in the text-phrasing-mask.
 *
 * @param lhs The text-phrasing-mask, i.e. the pattern.
 * @param rhs The text-phrasing.
 * @return True when the text-phrasing is part of the text-phrasing-mask.
 */
[[nodiscard]] constexpr bool matches(text_phrasing_mask const& lhs, text_phrasing const& rhs) noexcept
{
    return to_bool(lhs & to_text_phrasing_mask(rhs));
}

} // namespace hi::inline v1
