// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_general_category.hpp"
#include "unicode_bidi_class.hpp"
#include "unicode_bidi_bracket_type.hpp"
#include "unicode_grapheme_cluster_break.hpp"
#include "unicode_line_break.hpp"
#include "unicode_word_break.hpp"
#include "unicode_sentence_break.hpp"
#include "unicode_east_asian_width.hpp"
#include "unicode_decomposition_type.hpp"
#include "unicode_script.hpp"
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

constexpr char32_t unicode_replacement_character = U'\ufffd';
constexpr char32_t unicode_LS = U'\u2028';
constexpr char32_t unicode_PS = U'\u2029';

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
    constexpr unicode_description() noexcept = default;
    unicode_description(unicode_description const &) = delete;
    unicode_description &operator=(unicode_description const &) = delete;
    constexpr unicode_description(unicode_description &&) noexcept = default;
    constexpr unicode_description &operator=(unicode_description &&) noexcept = default;

    [[nodiscard]] constexpr unicode_description(
        char32_t code_point,
        unicode_general_category general_category,
        unicode_grapheme_cluster_break grapheme_cluster_break,
        unicode_line_break_class line_break_class,
        unicode_word_break_property word_break_property,
        unicode_sentence_break_property sentence_break_property,
        unicode_east_asian_width east_asian_width,
        unicode_script script,
        unicode_bidi_class bidi_class,
        unicode_bidi_bracket_type bidi_bracket_type,
        char32_t bidi_mirrored_glyph,
        unicode_decomposition_type decomposition_type,
        bool is_canonical_composition,
        uint8_t canonical_combining_class,
        uint8_t decomposition_length,
        uint32_t decomposition_index,
        uint16_t non_starter_code) noexcept :
        _general_info(
            (static_cast<uint32_t>(code_point) << code_point_shift) |
            (static_cast<uint32_t>(general_category) << general_category_shift) |
            (static_cast<uint32_t>(grapheme_cluster_break) << grapheme_cluster_break_shift) |
            (static_cast<uint32_t>(is_canonical_composition) << is_canonical_composition_shift) |
            (static_cast<uint32_t>(canonical_combining_class != 0) << is_combining_mark_shift)),
        _bidi_class(to_underlying(bidi_class)),
        _east_asian_width(static_cast<uint32_t>(east_asian_width)),
        _line_break_class(to_underlying(line_break_class)),
        _word_break_property(to_underlying(word_break_property)),
        _sentence_break_property(to_underlying(sentence_break_property)),
        _decomposition_index(static_cast<uint32_t>(decomposition_index)),
        _decomposition_type(static_cast<uint32_t>(decomposition_type)),
        _decomposition_length(static_cast<uint32_t>(decomposition_length))
    {
        // Check if the delta fits.
        if (canonical_combining_class == 0) {
            _non_mark.bidi_bracket_type = static_cast<uint32_t>(bidi_bracket_type);
            _non_mark.script = static_cast<uint32_t>(script);

            if (bidi_bracket_type != unicode_bidi_bracket_type::n and bidi_mirrored_glyph != char32_t{0xffff}) {
                auto mirrored_glyph_delta = static_cast<int32_t>(bidi_mirrored_glyph) - static_cast<int32_t>(code_point);
                tt_axiom(mirrored_glyph_delta >= bidi_mirrored_glyph_min and mirrored_glyph_delta <= bidi_mirrored_glyph_max);
                _non_mark.bidi_mirrored_glyph = static_cast<uint32_t>(mirrored_glyph_delta) & bidi_mirrored_glyph_mask;

            } else {
                _non_mark.bidi_mirrored_glyph = bidi_mirrored_glyph_null;
            }
            _non_mark._reserved = 0;

        } else {
            _mark.canonical_combining_class = canonical_combining_class;
            _mark.non_starter_code = static_cast<uint32_t>(non_starter_code);
            _mark._reserved = 0;
        }

        tt_axiom(code_point <= 0x10ffff);
        tt_axiom(to_underlying(general_category) <= 0x1f);
        tt_axiom(to_underlying(grapheme_cluster_break) <= 0x0f);
        tt_axiom(to_underlying(line_break_class) <= 0x3f);
        tt_axiom(to_underlying(word_break_property) <= 0x1f);
        tt_axiom(to_underlying(sentence_break_property) <= 0xf);
        tt_axiom(to_underlying(east_asian_width) <= 0x7);
        tt_axiom(to_underlying(script) <= 0xff);
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
        auto r = unicode_description{};
        r._general_info = other._general_info;
        r._bidi_class = other._bidi_class;
        if (other.is_combining_mark()) {
            r._mark = other._mark;
        } else {
            r._non_mark = other._non_mark;
        }
        r._east_asian_width = other._east_asian_width;
        r._line_break_class = other._line_break_class;
        r._word_break_property = other._word_break_property;
        r._sentence_break_property = other._sentence_break_property;
        r._decomposition_index = other._decomposition_index;
        r._decomposition_type = other._decomposition_type;
        r._decomposition_length = other._decomposition_length;

        r._general_info &= ~(general_category_mask << general_category_shift);
        r._general_info |= static_cast<uint32_t>(to_underlying(unicode_general_category::Cn)) << general_category_shift;
        return r;
    }

    /** The code point of the description.
     * @return The code_point of the description.
     */
    [[nodiscard]] constexpr char32_t code_point() const noexcept
    {
        return static_cast<char32_t>((_general_info >> code_point_shift) & code_point_mask);
    }

    /** The general category of this code-point.
     * This function is used to determine what kind of code-point this,
     * this allows you to determine if the code-point is a letter, number, punctuation, white-space, etc.
     *
     * @return The general category of this code-point
     */
    [[nodiscard]] constexpr unicode_general_category general_category() const noexcept
    {
        return static_cast<unicode_general_category>((_general_info >> general_category_shift) & general_category_mask);
    }

    /** The grapheme cluster break of this code-point.
     * This function is used to determine where to break a string of code-points
     * into grapheme clusters.
     *
     * @return The grapheme cluster break of this code-point
     */
    [[nodiscard]] constexpr unicode_grapheme_cluster_break grapheme_cluster_break() const noexcept
    {
        return static_cast<unicode_grapheme_cluster_break>(
            (_general_info >> grapheme_cluster_break_shift) & grapheme_cluster_break_mask);
    }

    [[nodiscard]] constexpr bool is_canonical_composition() const noexcept
    {
        return static_cast<bool>((_general_info >> is_canonical_composition_shift) & is_canonical_composition_mask);
    }

    [[nodiscard]] constexpr bool is_combining_mark() const noexcept
    {
        return static_cast<bool>((_general_info >> is_combining_mark_shift) & is_combining_mark_mask);
    }

    [[nodiscard]] constexpr unicode_line_break_class line_break_class() const noexcept
    {
        return static_cast<unicode_line_break_class>(_line_break_class);
    }

    [[nodiscard]] constexpr unicode_word_break_property word_break_property() const noexcept
    {
        return static_cast<unicode_word_break_property>(_word_break_property);
    }

    [[nodiscard]] constexpr unicode_sentence_break_property sentence_break_property() const noexcept
    {
        return static_cast<unicode_sentence_break_property>(_sentence_break_property);
    }

    [[nodiscard]] constexpr unicode_east_asian_width east_asian_width() const noexcept
    {
        return static_cast<unicode_east_asian_width>(_east_asian_width);
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
        if (is_combining_mark()) {
            return unicode_script::Common;
        } else {
            return static_cast<unicode_script>(_non_mark.script);
        }
    }

    /** Get the bidi bracket type.
     * This function is used by the bidirectional algorithm for mirroring characters
     * when needing to reverse the writing direction.
     *
     * @return n = no-mirror, o = open-bracket, c = close-bracket, m = bidi-mirrored.
     */
    [[nodiscard]] constexpr unicode_bidi_bracket_type bidi_bracket_type() const noexcept
    {
        if (is_combining_mark()) {
            return unicode_bidi_bracket_type::n;
        } else {
            return static_cast<unicode_bidi_bracket_type>(_non_mark.bidi_bracket_type);
        }
    }

    /** Get the mirrored glyph.
     * @return The mirrored glyph or U+ffff when there is no mirrored glyph.
     */
    [[nodiscard]] constexpr char32_t bidi_mirrored_glyph() const noexcept
    {
        if (bidi_bracket_type() == unicode_bidi_bracket_type::n or _non_mark.bidi_mirrored_glyph == bidi_mirrored_glyph_null) {
            return 0xffff;
        }

        constexpr auto sign_extent_shift = 32 - bidi_mirrored_glyph_width;

        ttlet mirrored_glyph_delta =
            static_cast<int32_t>(_non_mark.bidi_mirrored_glyph << sign_extent_shift) >> sign_extent_shift;
        ttlet cp = code_point();
        ttlet mirror_cp = static_cast<char32_t>(cp + mirrored_glyph_delta);
        return mirror_cp;
    }

    /** This character has a canonical decomposition.
     * @return When true you can decompose the character canonically.
     */
    [[nodiscard]] constexpr unicode_decomposition_type decomposition_type() const noexcept
    {
        return static_cast<unicode_decomposition_type>(_decomposition_type);
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
        if (is_combining_mark()) {
            return static_cast<uint8_t>(_mark.canonical_combining_class);
        } else {
            return 0;
        }
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
        tt_axiom(is_combining_mark());
        return static_cast<size_t>(_mark.non_starter_code);
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
    [[nodiscard]] static unicode_description const &find(char32_t code_point) noexcept;

    [[nodiscard]] friend bool operator==(unicode_description const &lhs, unicode_general_category const &rhs) noexcept
    {
        return lhs.general_category() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const &lhs, unicode_decomposition_type const &rhs) noexcept
    {
        return lhs.decomposition_type() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const &lhs, unicode_bidi_bracket_type const &rhs) noexcept
    {
        return lhs.bidi_bracket_type() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const &lhs, unicode_bidi_class const &rhs) noexcept
    {
        return lhs.bidi_class() == rhs;
    }
    [[nodiscard]] friend bool operator==(unicode_description const &lhs, unicode_east_asian_width const &rhs) noexcept
    {
        return lhs.east_asian_width() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const &lhs, unicode_sentence_break_property const &rhs) noexcept
    {
        return lhs.sentence_break_property() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const &lhs, unicode_line_break_class const &rhs) noexcept
    {
        return lhs.line_break_class() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const &lhs, unicode_word_break_property const &rhs) noexcept
    {
        return lhs.word_break_property() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const &lhs, unicode_grapheme_cluster_break const &rhs) noexcept
    {
        return lhs.grapheme_cluster_break() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const &lhs, char32_t const &rhs) noexcept
    {
        return lhs.code_point() == rhs;
    }

    [[nodiscard]] friend bool is_C(unicode_description const &rhs) noexcept
    {
        return is_C(rhs.general_category());
    }

private:
    static constexpr uint32_t code_point_shift = 11;
    static constexpr uint32_t code_point_mask = 0x1f'ffff;
    static constexpr uint32_t general_category_shift = 6;
    static constexpr uint32_t general_category_mask = 0x1f;
    static constexpr uint32_t grapheme_cluster_break_shift = 2;
    static constexpr uint32_t grapheme_cluster_break_mask = 0xf;
    static constexpr uint32_t is_canonical_composition_shift = 1;
    static constexpr uint32_t is_canonical_composition_mask = 0x1;
    static constexpr uint32_t is_combining_mark_shift = 0;
    static constexpr uint32_t is_combining_mark_mask = 0x1;

    static constexpr uint32_t bidi_mirrored_glyph_mask = 0x1fff;
    static constexpr uint32_t bidi_mirrored_glyph_width = std::bit_width(bidi_mirrored_glyph_mask);
    static constexpr int32_t bidi_mirrored_glyph_max = static_cast<int32_t>(bidi_mirrored_glyph_mask >> 1);
    static constexpr int32_t bidi_mirrored_glyph_min = -bidi_mirrored_glyph_max;
    static constexpr uint32_t bidi_mirrored_glyph_null =
        static_cast<uint32_t>(bidi_mirrored_glyph_min - 1) & bidi_mirrored_glyph_mask;

    // 1st dword
    // We don't use bit-fields so we can do binary-search without needing shift- & and-operations
    // code_point must be in msb for correct binary search.
    // [31:11] code-point
    // [10:6] general category
    // [5:2] grapheme cluster break
    // [1] is_canonical_composition
    // [0] is_combining_mark
    uint32_t _general_info;

    // 2nd dword
    uint32_t _bidi_class : 5;
    uint32_t _word_break_property : 5;
    uint32_t _line_break_class : 6;
    uint32_t _sentence_break_property : 4;
    uint32_t _word2_reserved : 12 = 0;

    struct mark_type {
        uint32_t canonical_combining_class : 8;
        uint32_t non_starter_code : 10;
        uint32_t _reserved : 14;
    };

    struct non_mark_type {
        uint32_t bidi_mirrored_glyph : 13;
        uint32_t bidi_bracket_type : 2;
        uint32_t script : 8;
        uint32_t _reserved : 9;
    };

    // 3rd dword
    union {
        mark_type _mark;
        non_mark_type _non_mark;
        uint32_t _word3 = 0;
    };

    // 4th dword
    uint32_t _decomposition_index : 21;
    uint32_t _decomposition_type : 3;
    uint32_t _decomposition_length : 5;
    uint32_t _east_asian_width : 3;
};

static_assert(sizeof(unicode_description) == 16);

} // namespace tt::inline v1
