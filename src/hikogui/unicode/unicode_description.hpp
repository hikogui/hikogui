// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_bidi_class.hpp"
#include "unicode_bidi_bracket_type.hpp"
#include "unicode_script.hpp"
#include "../utility/module.hpp"

namespace hi::inline v1 {

constexpr char32_t unicode_replacement_character = U'\ufffd';
constexpr char32_t unicode_LF = U'\n';
constexpr char32_t unicode_VT = U'\v';
constexpr char32_t unicode_FF = U'\f';
constexpr char32_t unicode_CR = U'\r';
constexpr char32_t unicode_NEL = U'\u0085';
constexpr char32_t unicode_LS = U'\u2028';
constexpr char32_t unicode_PS = U'\u2029';

/** Description of a unicode code point.
 * This class holds information of a unicode code point.
 *
 * The information is compressed to use bit-fields to reduce memory usage
 * of the unicode database.
 */
class unicode_description {
public:
    constexpr unicode_description() noexcept = default;
    unicode_description(unicode_description const&) = delete;
    unicode_description& operator=(unicode_description const&) = delete;
    constexpr unicode_description(unicode_description&&) noexcept = default;
    constexpr unicode_description& operator=(unicode_description&&) noexcept = default;

    [[nodiscard]] constexpr unicode_description(
        unicode_script script,
        unicode_bidi_class bidi_class,
        unicode_bidi_bracket_type bidi_bracket_type,
        char32_t bidi_mirroring_glyph) noexcept :
        _script(to_underlying(script)),
        _bidi_class(to_underlying(bidi_class)),
        _bidi_bracket_type(to_underlying(bidi_bracket_type)),
        _bidi_mirroring_glyph(truncate<uint32_t>(bidi_mirroring_glyph))
    {
        hi_assert(to_underlying(script) <= 0xff);
        hi_assert(to_underlying(bidi_class) <= 0x1f);
        hi_assert(to_underlying(bidi_bracket_type) <= 0x03);
        hi_assert(static_cast<uint32_t>(bidi_mirroring_glyph) <= 0xffff);
    }

    /** The bidi class of this code-point
     * This function is used by the bidirectional algorithm to figure out if the code-point
     * represents a character that is written left-to-right or right-to-left.
     *
     * @return the bidi class of this code-point.
     */
    [[nodiscard]] constexpr unicode_bidi_class bidi_class() const noexcept
    {
        return static_cast<unicode_bidi_class>(_bidi_class);
    }

    /** Get the script of this character.
     */
    [[nodiscard]] constexpr unicode_script script() const noexcept
    {
        return static_cast<unicode_script>(_script);
    }

    /** Get the bidi bracket type.
     * This function is used by the bidirectional algorithm for mirroring characters
     * when needing to reverse the writing direction.
     *
     * @return n = no-mirror, o = open-bracket, c = close-bracket, m = bidi-mirrored.
     */
    [[nodiscard]] constexpr unicode_bidi_bracket_type bidi_bracket_type() const noexcept
    {
        return static_cast<unicode_bidi_bracket_type>(_bidi_bracket_type);
    }

    /** Get the mirrored glyph.
     * @return The mirrored glyph or U+ffff when there is no mirrored glyph.
     */
    [[nodiscard]] constexpr char32_t bidi_mirroring_glyph() const noexcept
    {
        return truncate<char32_t>(_bidi_mirroring_glyph);
    }

    /** Find a code-point in the global unicode_description table.
     * For any valid unicode code point this function will return a reference to
     * the unicode_description. It may return a unicode_description to the
     * U+fffd 'REPLACEMENT CHARACTER' if the code-point could not be found in the
     * table. Or it may return unicode_description to a single element in a range
     * of code-points, such as for hangul-syllables, or private use areas.
     *
     * @param code_point The code point to look up.
     * @return a const reference to the unicode_description entry.
     */
    [[nodiscard]] static unicode_description const& find(char32_t code_point) noexcept;

    [[nodiscard]] friend bool operator==(unicode_description const& lhs, unicode_bidi_bracket_type const& rhs) noexcept
    {
        return lhs.bidi_bracket_type() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const& lhs, unicode_bidi_class const& rhs) noexcept
    {
        return lhs.bidi_class() == rhs;
    }

private:
    // 1st qword
    uint32_t _bidi_class : 5;
    uint32_t _bidi_bracket_type : 2;
    uint32_t _bidi_mirroring_glyph : 16;
    uint32_t _script : 8;
    uint32_t _word0_reserved : 1 = 0;
};

static_assert(sizeof(unicode_description) == 4);

} // namespace hi::inline v1
