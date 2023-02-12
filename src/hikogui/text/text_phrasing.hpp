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
    regular,

    /** Emphesised text; spoken as if the text is special importance, significant or promonent.
     * Often formatted in italic.
     */
    emphesis,

    /** Strong text; spoken louder, as if the text is not to be missed.
     * Often formatted in bold.
     */
    strong,

    /** Text is a piece of programming-code; a variable name, a function name.
     * Often formatted in a constant-width font, with a greater weight and in a different color and possible
     * background block, than the surrounding text.
     */
    code,

    /** An abbreviation.
     * Sometimes formatted with a double underline and hovering will show the expansion of the abbreviation.
     */
    abbreviation,

    /** Used to make text bold without it being semantically strong.
     */
    bold,

    /** Used to make text italic without it being semantically an emphesis.
     */
    italic,

    /** The text is quoted from somewhere.
     * Often formatted using a more italic / cursive font, with a lower weight.
     */
    citation,

    /** Used in help text to show which key or button to press.
     * Often formatted with a background that looks raised up like a button.
     * With the text in inverted color.
     */
    keyboard,

    /** The text is marked or highlighted as if being marked by a highlight pen.
     * Often formatted with a yellow background.
     */
    mark,

    /** Text formatted as math.
     * Often formatted using a special math font.
     */
    math,

    /** Used in help text to show an example.
     * Often formatted using a non-proportional font with a low resultion bitmap-like style.
     */
    example, ///< Used for displaying console output.

    /** Unarticulated.
     * Often formatted using an underlying line.
     */
    unarticulated,
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

[[nodiscard]] constexpr std::optional<text_phrasing> to_text_phrasing(char c)
{
    switch (c) {
    case 'r': return text_phrasing::regular;
    case 'e': return text_phrasing::emphesis;
    case 's': return text_phrasing::strong;
    case 'c': return text_phrasing::code;
    case 'a': return text_phrasing::abbreviation;
    case 'b': return text_phrasing::bold;
    case 'i': return text_phrasing::italic;
    case 'q': return text_phrasing::citation;
    case 'k': return text_phrasing::keyboard;
    case 'h': return text_phrasing::mark;
    case 'm': return text_phrasing::math;
    case 'x': return text_phrasing::example;
    case 'u': return text_phrasing::unarticulated;
    default: return std::nullopt;
    }
}

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
