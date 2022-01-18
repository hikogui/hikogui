// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_general_category.hpp"
#include "unicode_bidi_class.hpp"
#include "unicode_bidi_bracket_type.hpp"
#include "unicode_grapheme_cluster_break.hpp"
#include "unicode_line_break.hpp"
#include "unicode_east_asian_width.hpp"
#include "unicode_decomposition_type.hpp"
#include "../required.hpp"
#include "../assert.hpp"
#include "../cast.hpp"

namespace tt::inline v1 {
namespace detail {

constexpr char32_t unicode_hangul_S_base = U'\uac00';
constexpr char32_t unicode_hangul_L_base = U'\u1100';
constexpr char32_t unicode_hangul_V_base = U'\u1161';
constexpr char32_t unicode_hangul_T_base = U'\u11a7';
constexpr char32_t unicode_hangul_L_count = 19;
constexpr char32_t unicode_hangul_V_count = 21;
constexpr char32_t unicode_hangul_T_count = 28;
constexpr char32_t unicode_hangul_N_count = unicode_hangul_V_count * unicode_hangul_T_count;
constexpr char32_t unicode_hangul_S_count = unicode_hangul_L_count * unicode_hangul_N_count;
} // namespace detail

constexpr char32_t replacement_character = U'\ufffd';
constexpr char32_t line_separator_character = U'\u2028';
constexpr char32_t paragraph_separator_character = U'\u2029';

[[nodiscard]] constexpr bool is_hangul_L_part(char32_t code_point) noexcept
{
    return code_point >= detail::unicode_hangul_L_base &&
        code_point < (detail::unicode_hangul_L_base + detail::unicode_hangul_L_count);
}

[[nodiscard]] constexpr bool is_hangul_V_part(char32_t code_point) noexcept
{
    return code_point >= detail::unicode_hangul_V_base &&
        code_point < (detail::unicode_hangul_V_base + detail::unicode_hangul_V_count);
}

[[nodiscard]] constexpr bool is_hangul_T_part(char32_t code_point) noexcept
{
    return code_point >= detail::unicode_hangul_T_base &&
        code_point < (detail::unicode_hangul_T_base + detail::unicode_hangul_T_count);
}

[[nodiscard]] constexpr bool is_hangul_syllable(char32_t code_point) noexcept
{
    return code_point >= detail::unicode_hangul_S_base &&
        code_point < (detail::unicode_hangul_S_base + detail::unicode_hangul_S_count);
}

[[nodiscard]] constexpr bool is_hangul_LV_part(char32_t code_point) noexcept
{
    return is_hangul_syllable(code_point) && ((code_point - detail::unicode_hangul_S_base) % detail::unicode_hangul_T_count) == 0;
}

[[nodiscard]] constexpr bool is_hangul_LVT_part(char32_t code_point) noexcept
{
    return is_hangul_syllable(code_point) && ((code_point - detail::unicode_hangul_S_base) % detail::unicode_hangul_T_count) != 0;
}

/** Description of a unicode code point.
 * This class holds information of a unicode code point.
 *
 * The information is compressed to use bit-fields to reduce memory usage
 * of the unicode database.
 */
class unicode_description {
public:
    static constexpr uint32_t code_point_shift = 10;
    static constexpr uint32_t code_point_mask = 0x1f'ffff << code_point_shift;
    static constexpr uint32_t general_category_shift = 5;
    static constexpr uint32_t general_category_mask = 0x1f << general_category_shift;
    static constexpr uint32_t grapheme_cluster_break_shift = 1;
    static constexpr uint32_t grapheme_cluster_break_mask = 0xf << grapheme_cluster_break_shift;

    [[nodiscard]] constexpr unicode_description(
        char32_t code_point,
        unicode_general_category general_category,
        unicode_grapheme_cluster_break grapheme_cluster_break,
        unicode_line_break_class line_break_class,
        unicode_east_asian_width east_asian_width,
        unicode_bidi_class bidi_class,
        unicode_bidi_bracket_type bidi_bracket_type,
        char32_t bidi_mirrored_glyph,
        unicode_decomposition_type decomposition_type,
        bool composition_canonical,
        uint8_t canonical_combining_class,
        uint8_t decomposition_length,
        uint32_t decomposition_index,
        uint16_t non_starter_code) noexcept :
        _general_info(
            (static_cast<uint32_t>(code_point) << code_point_shift) |
            (static_cast<uint32_t>(general_category) << general_category_shift) |
            (static_cast<uint32_t>(grapheme_cluster_break) << grapheme_cluster_break_shift)),
        _bidi_class(to_underlying(bidi_class)),
        _bidi_bracket_type(to_underlying(bidi_bracket_type)),
        _bidi_mirrored_glyph(static_cast<uint32_t>(bidi_mirrored_glyph)),
        _east_asian_width(static_cast<uint32_t>(east_asian_width)),
        _canonical_combining_class(static_cast<uint32_t>(canonical_combining_class)),
        _composition_canonical(static_cast<uint32_t>(composition_canonical)),
        _line_break_class(to_underlying(line_break_class)),
        _decomposition_index(static_cast<uint32_t>(decomposition_index)),
        _decomposition_type(static_cast<uint32_t>(decomposition_type)),
        _decomposition_length(static_cast<uint32_t>(decomposition_length)),
        _non_starter_code(static_cast<uint32_t>(non_starter_code))
    {
        tt_axiom(code_point <= 0x10ffff);
        tt_axiom(to_underlying(general_category) <= 0x1f);
        tt_axiom(to_underlying(grapheme_cluster_break) <= 0x0f);
        tt_axiom(to_underlying(line_break_class) <= 0x3f);
        tt_axiom(to_underlying(east_asian_width) <= 0x7);
        tt_axiom(to_underlying(bidi_class) <= 0x1f);
        tt_axiom(to_underlying(bidi_bracket_type) <= 0x03);
        tt_axiom(static_cast<uint32_t>(bidi_mirrored_glyph) <= 0x10ffff);
        tt_axiom(static_cast<uint32_t>(canonical_combining_class) <= 0xff);
        tt_axiom(to_underlying(decomposition_type) <= 0x7);
        tt_axiom(static_cast<uint32_t>(decomposition_length) <= 0x1f);
        tt_axiom(static_cast<uint32_t>(decomposition_index) <= 0x1f'ffff);
        tt_axiom(static_cast<uint32_t>(non_starter_code) <= 0x3ff);
    }

    [[nodiscard]] static constexpr unicode_description make_unassigned(unicode_description const &other)
    {
        auto r = other;
        r._general_info &= other._general_info & ~general_category_mask;
        r._general_info |= static_cast<uint32_t>(to_underlying(unicode_general_category::Cn)) << general_category_shift;
        return r;
    }

    /** The code point of the description.
     * @return The code_point of the description.
     */
    [[nodiscard]] constexpr char32_t code_point() const noexcept
    {
        return static_cast<char32_t>(_general_info >> 10);
    }

    /** The grapheme cluster break of this code-point.
     * This function is used to determine where to break a string of code-points
     * into grapheme clusters.
     *
     * @return The grapheme cluster break of this code-point
     */
    [[nodiscard]] constexpr unicode_grapheme_cluster_break grapheme_cluster_break() const noexcept
    {
        return static_cast<unicode_grapheme_cluster_break>((_general_info >> 1) & 0xf);
    }

    [[nodiscard]] constexpr unicode_line_break_class line_break_class() const noexcept
    {
        return static_cast<unicode_line_break_class>(_line_break_class);
    }

    [[nodiscard]] constexpr unicode_east_asian_width east_asian_width() const noexcept
    {
        return static_cast<unicode_east_asian_width>(_east_asian_width);
    }

    /** The general category of this code-point.
     * This function is used to determine what kind of code-point this,
     * this allows you to determine if the code-point is a letter, number, punctuation, white-space, etc.
     *
     * @return The general category of this code-point
     */
    [[nodiscard]] constexpr unicode_general_category general_category() const noexcept
    {
        return static_cast<unicode_general_category>((_general_info >> 5) & 0x1f);
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
    [[nodiscard]] constexpr char32_t bidi_mirrored_glyph() const noexcept
    {
        return static_cast<char32_t>(_bidi_mirrored_glyph);
    }

    /** This character has a canonical decomposition.
     * @return When true you can decompose the character canonically.
     */
    [[nodiscard]] constexpr unicode_decomposition_type decomposition_type() const noexcept
    {
        return static_cast<unicode_decomposition_type>(_decomposition_type);
    }

    /** This character has a canonical composition.
     * @return When true the decomposition_index() points into the composition table.
     */
    [[nodiscard]] constexpr bool composition_canonical() const noexcept
    {
        return static_cast<bool>(_composition_canonical);
    }

    /** Get the combining class.
     * The combing class describes how a code-point combines with other code-points.
     * Specifically the value 0 means that the code-point is a starter character,
     * and the numeric value of the combing class determines the order of the
     * the code-points after a starter before trying to look up composition in the
     * composition table.
     *
     * @return The numeric combining class of this code point.
     */
    [[nodiscard]] constexpr uint8_t canonical_combining_class() const noexcept
    {
        return static_cast<uint8_t>(_canonical_combining_class);
    }

    /** The number of code-points the decomposed grapheme has.
     *
     * @retval 0 There is no decomposition
     * @retval 1 Decomposition is a single code-point, the `decomposition_index()` is
     *           the numeric value of the code point.
     * @retval 2 Decomposition is has two code-point. When composition_canonical is set the
     *           `decomposition_index()` points in the composition table. Otherwise the
     *           index points into the decomposition table.
     * @return Values 3 and above are the number of code points in the decomposition table
     *          pointed to from the `decomposition_index()`.
     */
    [[nodiscard]] constexpr std::size_t decomposition_length() const noexcept
    {
        return static_cast<std::size_t>(_decomposition_length);
    }

    /** A multi-use value representing the decomposition of this code-point.
     * To compress the data for decomposition:
     *  - For single code-point decomposition the index itself is the code-point value.
     *  - For double code-point decomposition, if it is equal to the composition it points
     *    into the composition table, otherwise it points into the decomposition table
     *  - Anything else points into the decomposition table.
     *
     * @see decomposition_length
     * @return A code-point value, or a index into the composition table, or an index into the
     *         decomposition table.
     */
    [[nodiscard]] constexpr std::size_t decomposition_index() const noexcept
    {
        return static_cast<std::size_t>(_decomposition_index);
    }

    /** Get the canonical equivalent of this code-point.
     * The canonical equivalent is the code-point after NFC-normalization.
     * This is equal to canonical decomposition to a single code-point.
     *
     * @return The canonical equivalent code-point or U+ffff if there is not equivalent.
     */
    [[nodiscard]] constexpr char32_t canonical_equivalent() const noexcept
    {
        if (decomposition_type() == unicode_decomposition_type::canonical and _decomposition_length == 1) {
            return static_cast<char32_t>(_decomposition_index);
        } else {
            return U'\uffff';
        }
    }

    /** Get the non-starter-code
     * 
     * Instead of using a full 21-bit code-point this 10-bit value is used to compress non-starter characters.
     * 
     * @note It is undefined-behavior to call this function on a starter-character.
     * @return 10 bit code value for a non-starter character.
     */
    [[nodiscard]] constexpr size_t non_starter_code() const noexcept
    {
        tt_axiom(_canonical_combining_class != 0);
        return static_cast<size_t>(_non_starter_code);
    }

private:
    // 1st dword
    // We don't use bit-fields so we can do binary-search without needing shift- & and-operations
    // code_point must be in msb for correct binary search.
    // [31:10] code-point
    // [9:5] general category
    // [4:1] grapheme cluster break
    // [0:0] reserved
    uint32_t _general_info;

    // 2nd dword
    uint32_t _bidi_class : 5;
    uint32_t _bidi_bracket_type : 2;
    uint32_t _bidi_mirrored_glyph : 21;
    uint32_t _east_asian_width : 3;
    uint32_t _word2_reserved : 1 = 0;

    // 3rd dword
    uint32_t _canonical_combining_class : 8;
    uint32_t _composition_canonical : 1;
    uint32_t _line_break_class : 6;
    uint32_t _non_starter_code : 10;
    uint32_t _word3_reserved : 7 = 0;

    // 4th dword
    uint32_t _decomposition_index : 21;
    uint32_t _decomposition_type : 3;
    uint32_t _decomposition_length : 5;
    uint32_t _word4_reserved : 3 = 0;

    template<typename It>
    friend constexpr It unicode_description_find(It first, It last, char32_t code_point) noexcept;
};

static_assert(sizeof(unicode_description) == 16);

/** Find a code-point in a unicode_description table using a binary-search algorithm.
 * @param first The iterator pointing to the first element of a sorted container of unicode_description objects.
 * @param last The iterator pointing to one beyond the last element of a sorted container of unicode_description objects.
 * @param code_point The code point to look up.
 * @return An iterator pointing the found unicode_description, or the last iterator when not found.
 */
template<typename It>
[[nodiscard]] constexpr It unicode_description_find(It first, It last, char32_t code_point) noexcept
{
    tt_axiom(code_point <= 0x10'ffff);
    uint32_t general_info = static_cast<uint32_t>(code_point) << 10;

    auto it = std::lower_bound(first, last, general_info, [](auto const &item, auto const &value) {
        return item._general_info < value;
    });

    if (it == last || it->code_point() != code_point) {
        return last;
    } else {
        return it;
    }
}

/** Find a code-point in the global unicode_description table.
 * For any valid unicode code point this function will return a reference to
 * the unicode_description. It may return a unicode_description to the
 * U+fffd 'REPLACEMENT CHARACTER' if the code-point could not be found in the
 * table. Or it may return unicode_description to a single element in a range
 * of code-points, such as for hangul-syllables, or private use areas..
 *
 * Passing an invalid unicode value causes undefined behaviour.
 *
 * @param code_point The code point to look up.
 * @return a const reference to the unicode_description entry.
 */
[[nodiscard]] unicode_description const &unicode_description_find(char32_t code_point) noexcept;

} // namespace tt::inline v1
