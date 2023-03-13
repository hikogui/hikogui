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
struct unicode_normalization_config {
    uint64_t decompose_canonical: 1 = 0;
    uint64_t decompose_font: 1 = 0;
    uint64_t decompose_no_break: 1 = 0;
    uint64_t decompose_initial: 1 = 0;
    uint64_t decompose_medial: 1 = 0;
    uint64_t decompose_final: 1 = 0;
    uint64_t decompose_isolated: 1 = 0;
    uint64_t decompose_circle: 1 = 0;
    uint64_t decompose_super: 1 = 0;
    uint64_t decompose_sub: 1 = 0;
    uint64_t decompose_fraction: 1 = 0;
    uint64_t decompose_vertical: 1 = 0;
    uint64_t decompose_wide: 1 = 0;
    uint64_t decompose_narrow: 1 = 0;
    uint64_t decompose_small: 1 = 0;
    uint64_t decompose_square: 1 = 0;
    uint64_t decompose_compat: 1 = 0;

    /** During decomposition remove control characters.
    *
    * This will also drop newline characters like CR, LF, CR+LF, NEL, VTAB & FF;
     * these may be retained by using newline_to_ and _is_newline.
     */
    uint64_t drop_C0: 1 = 0;
    uint64_t drop_C1: 1 = 0;

    /** Drop the LF character.
     */
    uint64_t drop_LF: 1 = 0;

    /** Drop the VT character.
     */
    uint64_t drop_VT: 1 = 0;

    /** Drop the FF character.
     */
    uint64_t drop_FF: 1 = 0;

    /** Drop the CR character.
     */
    uint64_t drop_CR: 1 = 0;

    /** Drop the NEL character.
     */
    uint64_t drop_NEL: 1 = 0;

    /** Drop the LS
     */
    uint64_t drop_LS: 1 = 0;

    /** Drop the PS character.
     */
    uint64_t drop_PS: 1 = 0;

    uint64_t LF_is_line_separator: 1 = 0;
    uint64_t VT_is_line_separator: 1 = 0;
    uint64_t FF_is_line_separator: 1 = 0;
    uint64_t CR_is_line_separator: 1 = 0;
    uint64_t NEL_is_line_separator: 1 = 0;
    uint64_t LS_is_line_separator: 1 = 0;
    uint64_t PS_is_line_separator: 1 = 0;

    uint64_t LF_is_paragraph_separator: 1 = 0;
    uint64_t VT_is_paragraph_separator: 1 = 0;
    uint64_t FF_is_paragraph_separator: 1 = 0;
    uint64_t CR_is_paragraph_separator: 1 = 0;
    uint64_t NEL_is_paragraph_separator: 1 = 0;
    uint64_t LS_is_paragraph_separator: 1 = 0;
    uint64_t PS_is_paragraph_separator: 1 = 0;

    /** The code-point to output when a line separator was found.
     *
     * If this value is the carriage return CR, then the decomposition algorithm will also emit a line-feed LF code-point.
     */
    char32_t line_separator_character = unicode_LS;

    /** The code-points to output when a paragraph separator was found.
     *
     * If this value is the carriage return CR, then the decomposition algorithm will also emit a line-feed LF code-point.
     */
    char32_t paragraph_separator_character = unicode_PS;

    [[nodiscard]] constexpr static NFD() noexcept
    {
        auto r = unicode_normalization_config();
        r.decompose_canonical = 1;
        return r;
    }

    [[nodiscard]] constexpr static NFKD() noexcept
    {
        auto r = unicode_normalization_config::NFD();
        r.decompose_font = 1;
        r.decompose_no_break = 1;
        r.decompose_initial = 1;
        r.decompose_medial = 1;
        r.decompose_final = 1;
        r.decompose_isolated = 1;
        r.decompose_circle = 1;
        r.decompose_super = 1;
        r.decompose_sub = 1;
        r.decompose_fraction = 1;
        r.decompose_vertical = 1;
        r.decompose_wide = 1;
        r.decompose_narrow = 1;
        r.decompose_small = 1;
        r.decompose_square = 1;
        r.decompose_compat = 1;
        return r;
    }
};

// clang-format on

/** Convert text to Unicode-NFD normal form.
 *
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param normalization_mask Extra features for normalization.
 */
[[nodiscard]] std::u32string
unicode_decompose(std::u32string_view text, unicode_normalization_mask normalization_mask = unicode_normalization_config::NFD()) noexcept;

/** Convert text to Unicode-NFC normal form.
 *
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param normalization_mask Extra features for normalization.
 */
[[nodiscard]] std::u32string
unicode_normalize(std::u32string_view text, unicode_normalization_mask normalization_mask = unicode_normalization_config::NFD()) noexcept;

} // namespace hi::inline v1
