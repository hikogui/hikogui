// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "unicode_general_category.hpp"
#include "unicode_bidi_class.hpp"
#include "unicode_bidi_bracket_type.hpp"
#include "unicode_grapheme_cluster_break.hpp"
#include "../required.hpp"

namespace tt {
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
}

[[nodiscard]] constexpr bool is_hangul_L_part(char32_t code_point) noexcept
{
    return code_point >= detail::unicode_hangul_L_base && code_point < (detail::unicode_hangul_L_base + detail::unicode_hangul_L_count);
}

[[nodiscard]] constexpr bool is_hangul_V_part(char32_t code_point) noexcept
{
    return code_point >= detail::unicode_hangul_V_base && code_point < (detail::unicode_hangul_V_base + detail::unicode_hangul_V_count);
}

[[nodiscard]] constexpr bool is_hangul_T_part(char32_t code_point) noexcept
{
    return code_point >= detail::unicode_hangul_T_base && code_point < (detail::unicode_hangul_T_base + detail::unicode_hangul_T_count);
}

[[nodiscard]] constexpr bool is_hangul_syllable(char32_t code_point) noexcept
{
    return code_point >= detail::unicode_hangul_S_base && code_point < (detail::unicode_hangul_S_base + detail::unicode_hangul_S_count);
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
    [[nodiscard]] constexpr unicode_description(
        char32_t code_point,
        unicode_general_category general_category,
        unicode_grapheme_cluster_break grapheme_cluster_break,
        unicode_bidi_class bidi_class,
        unicode_bidi_bracket_type bidi_bracket_type,
        char32_t bidi_mirrored_glyph,
        bool decomposition_canonical,
        bool composition_canonical,
        uint8_t combining_class,
        uint8_t decomposition_length,
        uint32_t decomposition_index
    ) noexcept :
        _general_info((static_cast<uint32_t>(code_point) << 10) | (static_cast<uint32_t>(general_category) << 5) | (static_cast<uint32_t>(grapheme_cluster_break) << 1)),
        _bidi_class(static_cast<uint32_t>(bidi_class)),
        _bidi_bracket_type(static_cast<uint32_t>(bidi_bracket_type)),
        _bidi_mirrored_glyph(static_cast<uint32_t>(bidi_mirrored_glyph)),
        _decomposition_canonical(static_cast<uint32_t>(decomposition_canonical)),
        _composition_canonical(static_cast<uint32_t>(composition_canonical)),
        _combining_class(static_cast<uint32_t>(combining_class)),
        _decomposition_index(static_cast<uint32_t>(decomposition_index)),
        _decomposition_length(static_cast<uint32_t>(decomposition_length))
    {
        tt_axiom(code_point <= 0x10ffff);
        tt_axiom(static_cast<uint32_t>(general_category) <= 0x1f);
        tt_axiom(static_cast<uint32_t>(grapheme_cluster_break) <= 0x0f);
        tt_axiom(static_cast<uint32_t>(bidi_class) <= 0x1f);
        tt_axiom(static_cast<uint32_t>(bidi_bracket_type) <= 0x03);
        tt_axiom(static_cast<uint32_t>(bidi_mirrored_glyph) <= 0x10ffff);
        tt_axiom(static_cast<uint32_t>(combining_class) <= 0xff);
        tt_axiom(static_cast<uint32_t>(decomposition_length) <= 0x1f);
        tt_axiom(static_cast<uint32_t>(decomposition_index) <= 0x1f'ffff);
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
        return static_cast<char32_t>(_bidi_bracket_type);
    }

    /** This character has a canonical decomposition.
     * @return When true you can decompose the character canonically.
     */
    [[nodiscard]] constexpr bool decomposition_canonical() const noexcept
    {
        return static_cast<bool>(_decomposition_canonical);
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
    [[nodiscard]] constexpr uint8_t combining_class() const noexcept
    {
        return static_cast<uint8_t>(_combining_class);
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
    [[nodiscard]] constexpr size_t decomposition_length() const noexcept
    {
        return static_cast<size_t>(_decomposition_length);
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
    [[nodiscard]] constexpr size_t decomposition_index() const noexcept
    {
        return static_cast<size_t>(_decomposition_index);
    }

private:
    // 1st dword
    // code_point must be in msb for fast binary search, so no bit-fields here.
    // [31:10] code-point
    // [9:5] general category
    // [4:1] grapheme cluster break
    // [0:0] reserved
    uint32_t _general_info;

    // 2nd dword
    uint32_t _bidi_class:5;
    uint32_t _bidi_bracket_type:2;
    uint32_t _bidi_mirrored_glyph : 21;
    uint32_t _bidi_reserved : 4 = 0;

    // 3rd dword
    uint32_t _decomposition_canonical : 1;
    uint32_t _composition_canonical : 1;
    uint32_t _combining_class : 8;
    uint32_t _decomposition_index : 21;
    uint32_t _decomposition_reserved1 : 1 = 0;

    // 4th dword
    uint32_t _decomposition_length : 5;
    uint32_t _decomposition_reserved2 : 27 = 0;


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
 * @param code-point The code point to look up.
 * @return a const reference to the unicode_description entry.
 */
[[nodiscard]] unicode_description const &unicode_description_find(char32_t code_point) noexcept;

}
