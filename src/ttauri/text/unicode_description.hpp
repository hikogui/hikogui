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
        uint8_t decomposition_combining_class,
        uint8_t decomposition_length,
        uint32_t decomposition_index
    ) noexcept :
        _general_info((static_cast<uint32_t>(code_point) << 10) | (static_cast<uint32_t>(general_category) << 5) | (static_cast<uint32_t>(grapheme_cluster_break) << 1)),
        _bidi_class(static_cast<uint32_t>(bidi_class)),
        _bidi_bracket_type(static_cast<uint32_t>(bidi_bracket_type)),
        _bidi_mirrored_glyph(static_cast<uint32_t>(bidi_mirrored_glyph)),
        _decomposition_canonical(static_cast<uint32_t>(decomposition_canonical)),
        _decomposition_combining_class(static_cast<uint32_t>(decomposition_combining_class)),
        _decomposition_index(static_cast<uint32_t>(decomposition_index)),
        _decomposition_length(static_cast<uint32_t>(decomposition_length))
    {
        tt_axiom(code_point <= 0x10ffff);
        tt_axiom(static_cast<uint32_t>(general_category) <= 0x1f);
        tt_axiom(static_cast<uint32_t>(grapheme_cluster_break) <= 0x0f);
        tt_axiom(static_cast<uint32_t>(bidi_class) <= 0x1f);
        tt_axiom(static_cast<uint32_t>(bidi_bracket_type) <= 0x03);
        tt_axiom(static_cast<uint32_t>(bidi_mirrored_glyph) <= 0x10ffff);
        tt_axiom(static_cast<uint32_t>(decomposition_combining_class) <= 0xff);
        tt_axiom(static_cast<uint32_t>(decomposition_length) <= 0x1f);
        tt_axiom(static_cast<uint32_t>(decomposition_index) <= 0x1f'ffff);
    }

    [[nodiscard]] constexpr char32_t code_point() const noexcept
    {
        return static_cast<char32_t>(_general_info >> 10);
    }

    [[nodiscard]] constexpr unicode_grapheme_cluster_break grapheme_cluster_break() const noexcept
    {
        return static_cast<unicode_grapheme_cluster_break>((_general_info >> 1) & 0xf);
    }

    [[nodiscard]] constexpr unicode_general_category general_category() const noexcept
    {
        return static_cast<unicode_general_category>((_general_info >> 5) & 0x1f);
    }

    [[nodiscard]] constexpr unicode_bidi_class bidi_class() const noexcept
    {
        return static_cast<unicode_bidi_class>(_bidi_class);
    }

    /** Get the bidi bracket type.
     * The returned type combined the bracket type and is-bidi-mirrored property.
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
        return static_cast<unicode_bidi_bracket_type>(_bidi_bracket_type);
    }

    [[nodiscard]] constexpr bool decomposition_canonical() const noexcept
    {
        return static_cast<bool>(_composition_canonical);
    }

    [[nodiscard]] constexpr uint8_t decomposition_combining_class() const noexcept
    {
        return static_cast<uint8_t>(_decomposition_combining_class);
    }

    [[nodiscard]] constexpr size_t decomposition_index() const noexcept
    {
        tt_axiom(decomposition_length() > 1);
        return static_cast<size_t>(_decomposition_index);
    }

    [[nodiscard]] constexpr char32_t decomposition_single_char() const noexcept
    {
        tt_axiom(decomposition_length() == 1);
        return static_cast<char32_t>(_decomposition_index);
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
    uint32_t _decomposition_canonical:1;
    uint32_t _decomposition_combining_class : 8;
    uint32_t _decomposition_index : 21;
    uint32_t _decomposition_reserved1 : 2 = 0;

    // 4th dword
    uint32_t _decomposition_length : 5;
    uint32_t _decomposition_reserved2 : 27 = 0;

    template<typename It>
    friend constexpr It unicode_description_find(It first, It last, char32_t code_point) noexcept;
};

static_assert(sizeof(unicode_description) == 16);

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

[[nodiscard]] unicode_description const &unicode_description_find(char32_t code_point) noexcept;

}
