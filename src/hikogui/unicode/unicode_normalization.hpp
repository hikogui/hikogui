// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_decomposition_type.hpp"
#include "../utility/module.hpp"
#include "../algorithm.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace hi::inline v1 {

// Windows.h defines small as a macro.
#ifdef small
#undef small
#endif

// clang-format off
enum class unicode_normalization_mask: unsigned long long {
    decompose_canonical = 1ULL << to_underlying(unicode_decomposition_type::canonical),
    decompose_font = 1ULL << to_underlying(unicode_decomposition_type::font),
    decompose_noBreak = 1ULL << to_underlying(unicode_decomposition_type::noBreak),
    decompose_initial = 1ULL << to_underlying(unicode_decomposition_type::initial),
    decompose_medial = 1ULL << to_underlying(unicode_decomposition_type::medial),
    decompose_final = 1ULL << to_underlying(unicode_decomposition_type::_final),
    decompose_isolated = 1ULL << to_underlying(unicode_decomposition_type::isolated),
    decompose_circle = 1ULL << to_underlying(unicode_decomposition_type::circle),
    decompose_super = 1ULL << to_underlying(unicode_decomposition_type::super),
    decompose_sub = 1ULL << to_underlying(unicode_decomposition_type::sub),
    decompose_fraction = 1ULL << to_underlying(unicode_decomposition_type::fraction),
    decompose_vertical = 1ULL << to_underlying(unicode_decomposition_type::vertical),
    decompose_wide = 1ULL << to_underlying(unicode_decomposition_type::wide),
    decompose_narrow = 1ULL << to_underlying(unicode_decomposition_type::narrow),
    decompose_small = 1ULL << to_underlying(unicode_decomposition_type::small),
    decompose_square = 1ULL << to_underlying(unicode_decomposition_type::square),
    decompose_compat = 1ULL << to_underlying(unicode_decomposition_type::compat),

    /** During decomposition remove control characters.
    *
    * This will also drop newline characters like CR, LF, CR+LF, NEL, VTAB & FF;
     * these may be retained by using newline_to_ and _is_newline.
     */
    drop_control = 1ULL << 25,

    /** Drop the LF character.
     */
    drop_LF = 1ULL << 26,

    /** Drop the VT character.
     */
    drop_VT = 1ULL << 26,

    /** Drop the VT character.
     */
    drop_VT = 1ULL << 26,

    /** Drop the CR character.
     */
    drop_CR = 1ULL << 26,

    LF_is_newline = 1ULL << 27,
    VT_is_newline = 1ULL << 28,
    FF_is_newline = 1ULL << 29,
    CR_is_newline = 1ULL << 30,
    NEL_is_newline = 1ULL << 31,
    LS_is_newline = 1ULL << 32,
    PS_is_newline = 1ULL << 33,

    /** Decompose any newline character into PS (Paragraph Separator).
     *
     * @note Mutually exclusive with decompose_LF, decompose_CRLF and decompose_newline_to_SP.
     */
    newline_to_PS = 0ULL << 63,

    /** Decompose any newline character into LF (Line Feed).
     *
     * @note Mutually exclusive with decompose_PS, decompose_CRLF and decompose_newline_to_SP.
     */
    newline_to_LF = 1ULL << 63,

    /** Decompose any newline character into CR+LF (Carriage Return + Line Feed).
     *
     * @note Mutually exclusive with decompose_PS, decompose_LF and decompose_newline_to_SP.
     */
    newline_to_CRLF = 2ULL << 63,

    /** Decompose any newline character into SP (Space).
     *
     * @note Mutually exclusive with decompose_PS, decompose_newline_to_CRLF and decompose_LF.
     */
    newline_to_SP = 3ULL << 63,

    /** Mask for one of the newline_to_*
     *
     * @note Only one of newline_to_* may be used.
     */
    newline_to_mask = newline_to_PS | newline_to_LF | newline_to_CRLF | newline_to_SP,

    /** Canonical decomposition and composition.
     */
    NFD = decompose_canonical,

    /** Compatible decomposition and composition.
     */
    NFKD =
        NFD | decompose_font | decompose_noBreak | decompose_initial | decompose_medial | decompose_final | decompose_isolated |
        decompose_circle | decompose_super | decompose_sub | decompose_fraction | decompose_vertical | decompose_wide |
        decompose_narrow | decompose_small | decompose_square | decompose_compat,
};
// clang-format on

[[nodiscard]] constexpr bool to_bool(unicode_normalization_mask const& rhs) noexcept
{
    return to_bool(to_underlying(rhs));
}

[[nodiscard]] constexpr unicode_normalization_mask
operator|(unicode_normalization_mask const& lhs, unicode_normalization_mask const& rhs) noexcept
{
    return static_cast<unicode_normalization_mask>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr unicode_normalization_mask
operator&(unicode_normalization_mask const& lhs, unicode_normalization_mask const& rhs) noexcept
{
    return static_cast<unicode_normalization_mask>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr unicode_normalization_mask
operator&(unicode_normalization_mask const& lhs, unicode_decomposition_type const& rhs) noexcept
{
    return static_cast<unicode_normalization_mask>(to_underlying(lhs) & (1 << to_underlying(rhs)));
}

/** Convert text to Unicode-NFD normal form.
 *
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param normalization_mask Extra features for normalization.
 */
[[nodiscard]] std::u32string
unicode_decompose(std::u32string_view text, unicode_normalization_mask normalization_mask = unicode_normalization_mask::NFD) noexcept;

/** Convert text to Unicode-NFC normal form.
 *
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param normalization_mask Extra features for normalization.
 */
[[nodiscard]] std::u32string
unicode_normalize(std::u32string_view text, unicode_normalization_mask normalization_mask = unicode_normalization_mask::NFD) noexcept;

} // namespace hi::inline v1
