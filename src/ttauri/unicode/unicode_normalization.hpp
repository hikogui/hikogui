// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_decomposition_type.hpp"
#include "../cast.hpp"
#include "../algorithm.hpp"
#include <cstdint>
#include <string>
#include <string_view>

namespace tt::inline v1 {

enum class unicode_normalization_mask {
    decompose_canonical = 1 << to_underlying(unicode_decomposition_type::canonical),
    decompose_font = 1 << to_underlying(unicode_decomposition_type::font),
    decompose_no_break = 1 << to_underlying(unicode_decomposition_type::no_break),
    decompose_arabic = 1 << to_underlying(unicode_decomposition_type::arabic),
    decompose_circle = 1 << to_underlying(unicode_decomposition_type::circle),
    decompose_math = 1 << to_underlying(unicode_decomposition_type::math),
    decompose_asian = 1 << to_underlying(unicode_decomposition_type::asian),
    decompose_compat = 1 << to_underlying(unicode_decomposition_type::compat),

    decompose_hangul = 0x100,
    compose_hangul = 0x200,

    decompose_newline = 0x7000, ///< Mask for one of decompose_PS, decompose_LF, decompose_CR, decompose_CR_LF
    decompose_PS = 0x1000, ///< Decompose newline -> PS
    decompose_LF = 0x2000, ///< Decompose newline -> LF
    decompose_CR_LF = 0x2000, ///< Decompose newline -> CR+LF.
    compose_CR_LF = 0x8000, ///< Compose CR+LF -> LF


    NFD = decompose_canonical | decompose_hangul | compose_hangul,
    NFKD = NFD | decompose_font | decompose_no_break | decompose_arabic | decompose_circle | decompose_math | decompose_asian | decompose_compat,
};

[[nodiscard]] constexpr bool any(unicode_normalization_mask const &rhs) noexcept
{
    return static_cast<bool>(to_underlying(rhs));
}

[[nodiscard]] constexpr unicode_normalization_mask
operator|(unicode_normalization_mask const &lhs, unicode_normalization_mask const &rhs) noexcept
{
    return static_cast<unicode_normalization_mask>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr unicode_normalization_mask
operator&(unicode_normalization_mask const &lhs, unicode_normalization_mask const &rhs) noexcept
{
    return static_cast<unicode_normalization_mask>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr unicode_normalization_mask operator&(unicode_normalization_mask const &lhs, unicode_decomposition_type const &rhs) noexcept
{
    return static_cast<unicode_normalization_mask>(to_underlying(rhs) & (1 << to_underlying(rhs)));
}

/** Convert text to Unicode-NFD normal form.
 *
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param normalization_mask Extra features for normalization.
 */
std::u32string
unicode_NFD(std::u32string_view text, unicode_normalization_mask normalization_mask = unicode_normalization_mask::NFD) noexcept;

/** Convert text to Unicode-NFC normal form.
 *
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param normalization_mask Extra features for normalization.
 */
[[nodiscard]] std::u32string
unicode_NFC(std::u32string_view text, unicode_normalization_mask normalization_mask = unicode_normalization_mask::NFD) noexcept;

/** Convert text to Unicode-NFKD normal form.
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param normalization_mask Extra features for normalization.
 */
std::u32string
unicode_NFKD(std::u32string_view text, unicode_normalization_mask normalization_mask = unicode_normalization_mask::NFKD) noexcept;

/** Convert text to Unicode-NFKC normal form.
 * Code point 0x00'ffff is used internally, do not pass in text.
 *
 * @param text to normalize, in-place.
 * @param normalization_mask Extra features for normalization.
 */
std::u32string
unicode_NFKC(std::u32string_view text, unicode_normalization_mask normalization_mask = unicode_normalization_mask::NFKD) noexcept;

} // namespace tt::inline v1
