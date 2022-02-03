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
    canonical = 1 << to_underlying(unicode_decomposition_type::canonical),
    font = 1 << to_underlying(unicode_decomposition_type::font),
    no_break = 1 << to_underlying(unicode_decomposition_type::no_break),
    arabic = 1 << to_underlying(unicode_decomposition_type::arabic),
    circle = 1 << to_underlying(unicode_decomposition_type::circle),
    math = 1 << to_underlying(unicode_decomposition_type::math),
    asian = 1 << to_underlying(unicode_decomposition_type::asian),
    compat = 1 << to_underlying(unicode_decomposition_type::compat),

    paragraph = 0x0100, ///< Decompose LF -> PS (paragraph separator), Compose CR LF -> PS
    line_feed = 0x0200, ///< Decompose/Compose LS -> SP and PS -> LF 
    hangul = 0x0400, ///< Decompose/Compose hangul

    NFD = canonical | hangul,
    NFKD = NFD | font | no_break | arabic | circle | math | asian | compat,
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

[[nodiscard]] constexpr bool operator==(unicode_decomposition_type const &lhs, unicode_normalization_mask const &rhs) noexcept
{
    return static_cast<bool>((1 << to_underlying(lhs)) & to_underlying(rhs));
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
