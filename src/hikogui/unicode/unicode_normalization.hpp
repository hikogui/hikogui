// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "ucd_normalize.hpp"
#include "unicode_description.hpp"
#include "../utility/module.hpp"
#include "../algorithm.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace hi::inline v1 {

struct unicode_normalization_config {
    /** The types of decompositions, that should be used when decomposing.
     */
    uint64_t decomposition_mask : 17 = 0;

    /** Drop the C0 control characters.
     */
    uint64_t drop_C0 : 1 = 0;

    /** Drop the C1 control characters.
     */
    uint64_t drop_C1 : 1 = 0;

    /** Drop the LF character.
     */
    uint64_t drop_LF : 1 = 0;

    /** Drop the VT character.
     */
    uint64_t drop_VT : 1 = 0;

    /** Drop the FF character.
     */
    uint64_t drop_FF : 1 = 0;

    /** Drop the CR character.
     */
    uint64_t drop_CR : 1 = 0;

    /** Drop the NEL character.
     */
    uint64_t drop_NEL : 1 = 0;

    /** Drop the LS
     */
    uint64_t drop_LS : 1 = 0;

    /** Drop the PS character.
     */
    uint64_t drop_PS : 1 = 0;

    uint64_t LF_is_line_separator : 1 = 0;
    uint64_t VT_is_line_separator : 1 = 0;
    uint64_t FF_is_line_separator : 1 = 0;
    uint64_t CR_is_line_separator : 1 = 0;
    uint64_t NEL_is_line_separator : 1 = 0;
    uint64_t LS_is_line_separator : 1 = 0;
    uint64_t PS_is_line_separator : 1 = 0;

    uint64_t LF_is_paragraph_separator : 1 = 0;
    uint64_t VT_is_paragraph_separator : 1 = 0;
    uint64_t FF_is_paragraph_separator : 1 = 0;
    uint64_t CR_is_paragraph_separator : 1 = 0;
    uint64_t NEL_is_paragraph_separator : 1 = 0;
    uint64_t LS_is_paragraph_separator : 1 = 0;
    uint64_t PS_is_paragraph_separator : 1 = 0;

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

    constexpr unicode_normalization_config& add(unicode_decomposition_type type) noexcept
    {
        decomposition_mask |= 1_uz << to_underlying(type);
        return *this;
    }

    [[nodiscard]] constexpr static unicode_normalization_config NFD() noexcept
    {
        auto r = unicode_normalization_config();
        r.add(unicode_decomposition_type::canonical);
        return r;
    }

    [[nodiscard]] constexpr static unicode_normalization_config NFC() noexcept
    {
        return NFD();
    }

    /** Use NFC normalization, convert all line-feed-like characters to PS.
     */
    [[nodiscard]] constexpr static unicode_normalization_config NFC_PS_noctr() noexcept
    {
        auto r = NFC();
        r.drop_C0 = 1;
        r.drop_C1 = 1;
        r.drop_CR = 1;
        r.LF_is_paragraph_separator = 1;
        r.VT_is_paragraph_separator = 1;
        r.FF_is_paragraph_separator = 1;
        r.NEL_is_paragraph_separator = 1;
        r.LS_is_paragraph_separator = 1;
        r.PS_is_paragraph_separator = 1;
        return r;
    }

    /** Use NFC normalization, convert all line-feed-like characters to CR-LF.
     */
    [[nodiscard]] constexpr static unicode_normalization_config NFC_CRLF_noctr() noexcept
    {
        auto r = NFC();
        r.drop_C0 = 1;
        r.drop_C1 = 1;
        r.drop_CR = 1;
        r.LF_is_paragraph_separator = 1;
        r.VT_is_paragraph_separator = 1;
        r.FF_is_paragraph_separator = 1;
        r.NEL_is_paragraph_separator = 1;
        r.LS_is_paragraph_separator = 1;
        r.PS_is_paragraph_separator = 1;
        r.paragraph_separator_character = U'\r';
        return r;
    }

    [[nodiscard]] constexpr static unicode_normalization_config NFKD() noexcept
    {
        auto r = unicode_normalization_config::NFD();
        r.add(unicode_decomposition_type::canonical);
        r.add(unicode_decomposition_type::font);
        r.add(unicode_decomposition_type::no_break);
        r.add(unicode_decomposition_type::initial);
        r.add(unicode_decomposition_type::medial);
        r.add(unicode_decomposition_type::_final);
        r.add(unicode_decomposition_type::isolated);
        r.add(unicode_decomposition_type::circle);
        r.add(unicode_decomposition_type::super);
        r.add(unicode_decomposition_type::sub);
        r.add(unicode_decomposition_type::fraction);
        r.add(unicode_decomposition_type::vertical);
        r.add(unicode_decomposition_type::wide);
        r.add(unicode_decomposition_type::narrow);
        r.add(unicode_decomposition_type::small);
        r.add(unicode_decomposition_type::square);
        r.add(unicode_decomposition_type::compat);
        return r;
    }

    [[nodiscard]] constexpr static unicode_normalization_config NFKC() noexcept
    {
        return NFKD();
    }
};

/** Convert text to Unicode-NFD normal form.
 *
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param normalization_mask Extra features for normalization.
 */
[[nodiscard]] std::u32string
unicode_decompose(std::u32string_view text, unicode_normalization_config config = unicode_normalization_config::NFD()) noexcept;

/** Convert text to Unicode-NFC normal form.
 *
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param normalization_mask Extra features for normalization.
 */
[[nodiscard]] std::u32string
unicode_normalize(std::u32string_view text, unicode_normalization_config config = unicode_normalization_config::NFC()) noexcept;

} // namespace hi::inline v1
