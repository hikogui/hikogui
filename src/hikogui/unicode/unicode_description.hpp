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
        unicode_general_category general_category,
        unicode_grapheme_cluster_break grapheme_cluster_break,
        unicode_line_break_class line_break_class,
        unicode_word_break_property word_break_property,
        unicode_sentence_break_property sentence_break_property,
        unicode_east_asian_width east_asian_width,
        unicode_script script,
        unicode_bidi_class bidi_class,
        unicode_bidi_bracket_type bidi_bracket_type,
        char32_t bidi_mirroring_glyph) noexcept :
        _general_category(to_underlying(general_category)),
        _grapheme_cluster_break(to_underlying(grapheme_cluster_break)),
        _line_break_class(to_underlying(line_break_class)),
        _word_break_property(to_underlying(word_break_property)),
        _sentence_break_property(to_underlying(sentence_break_property)),
        _east_asian_width(to_underlying(east_asian_width)),
        _script(to_underlying(script)),
        _bidi_class(to_underlying(bidi_class)),
        _bidi_bracket_type(to_underlying(bidi_bracket_type)),
        _bidi_mirroring_glyph(truncate<uint32_t>(bidi_mirroring_glyph))
    {
        hi_assert(to_underlying(general_category) <= 0x1f);
        hi_assert(to_underlying(grapheme_cluster_break) <= 0x0f);
        hi_assert(to_underlying(line_break_class) <= 0x3f);
        hi_assert(to_underlying(word_break_property) <= 0x1f);
        hi_assert(to_underlying(sentence_break_property) <= 0xf);
        hi_assert(to_underlying(east_asian_width) <= 0x7);
        hi_assert(to_underlying(script) <= 0xff);
        hi_assert(to_underlying(bidi_class) <= 0x1f);
        hi_assert(to_underlying(bidi_bracket_type) <= 0x03);
        hi_assert(static_cast<uint32_t>(bidi_mirroring_glyph) <= 0xffff);
    }

    /** The general category of this code-point.
     * This function is used to determine what kind of code-point this,
     * this allows you to determine if the code-point is a letter, number, punctuation, white-space, etc.
     *
     * @return The general category of this code-point
     */
    [[nodiscard]] constexpr unicode_general_category general_category() const noexcept
    {
        return static_cast<unicode_general_category>(_general_category);
    }

    /** The grapheme cluster break of this code-point.
     * This function is used to determine where to break a string of code-points
     * into grapheme clusters.
     *
     * @return The grapheme cluster break of this code-point
     */
    [[nodiscard]] constexpr unicode_grapheme_cluster_break grapheme_cluster_break() const noexcept
    {
        return static_cast<unicode_grapheme_cluster_break>(_grapheme_cluster_break);
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

    [[nodiscard]] friend bool operator==(unicode_description const& lhs, unicode_general_category const& rhs) noexcept
    {
        return lhs.general_category() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const& lhs, unicode_bidi_bracket_type const& rhs) noexcept
    {
        return lhs.bidi_bracket_type() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const& lhs, unicode_bidi_class const& rhs) noexcept
    {
        return lhs.bidi_class() == rhs;
    }
    [[nodiscard]] friend bool operator==(unicode_description const& lhs, unicode_east_asian_width const& rhs) noexcept
    {
        return lhs.east_asian_width() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const& lhs, unicode_sentence_break_property const& rhs) noexcept
    {
        return lhs.sentence_break_property() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const& lhs, unicode_line_break_class const& rhs) noexcept
    {
        return lhs.line_break_class() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const& lhs, unicode_word_break_property const& rhs) noexcept
    {
        return lhs.word_break_property() == rhs;
    }

    [[nodiscard]] friend bool operator==(unicode_description const& lhs, unicode_grapheme_cluster_break const& rhs) noexcept
    {
        return lhs.grapheme_cluster_break() == rhs;
    }

    [[nodiscard]] friend bool is_C(unicode_description const& rhs) noexcept
    {
        return is_C(rhs.general_category());
    }

    [[nodiscard]] friend bool is_M(unicode_description const& rhs) noexcept
    {
        return is_M(rhs.general_category());
    }

private:
    // 1st qword
    uint64_t _general_category : 5;
    uint64_t _grapheme_cluster_break : 4;
    uint64_t _line_break_class : 6;
    uint64_t _word_break_property : 5;
    uint64_t _sentence_break_property : 4;
    uint64_t _east_asian_width : 3;
    uint64_t _bidi_class : 5;
    uint64_t _bidi_bracket_type : 2;
    uint64_t _bidi_mirroring_glyph : 16;
    uint64_t _script : 8;
    uint64_t _word0_reserved : 6 = 0;
};

static_assert(sizeof(unicode_description) == 8);

} // namespace hi::inline v1
