// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../cast.hpp"
#include <cstdint>

namespace tt::inline v1{

enum class unicode_decomposition_type : uint8_t {
    canonical = 0, ///< Canonical decomposition.
    font = 1, ///< <font> Font variant (for example, a blackletter form).
    no_break = 2, ///< <no_break> No - break version of a space or hyphen.
    arabic = 3, ///< <initial> <medial> <final> <isolated> Arabic presentation forms.
    circle = 4, ///< <circle> Encircled form.
    math = 5, ///< <super> <sub> <fraction> Super-, sub-script and Vulgar-fraction forms
    asian = 6, ///< <vertical> <wide> <narrow> <small> <square> asian compatibility forms.
    compat = 7 ///< <compat> Otherwise unspecified compatibility character
};

enum class unicode_normalization_mask : uint16_t {
    canonical = 1 << to_underlying(unicode_decomposition_type::canonical),
    font = 1 << to_underlying(unicode_decomposition_type::font),
    no_break = 1 << to_underlying(unicode_decomposition_type::no_break),
    arabic = 1 << to_underlying(unicode_decomposition_type::arabic),
    circle = 1 << to_underlying(unicode_decomposition_type::circle),
    math = 1 << to_underlying(unicode_decomposition_type::math),
    asian = 1 << to_underlying(unicode_decomposition_type::asian),
    compat = 1 << to_underlying(unicode_decomposition_type::compat),

    paragraph = 0x0100, ///< Decompose LF -> PS (paragraph separator), Compose CR LF -> PS
    hangul = 0x0200, ///< Decompose/Compose hangul

    NFD = canonical | hangul,
    NFKD = NFD | font | no_break | arabic | circle | math | asian | compat,
};

[[nodiscard]] constexpr bool any(unicode_normalization_mask const& rhs) noexcept
{
    return static_cast<bool>(to_underlying(rhs));
}

[[nodiscard]] constexpr unicode_normalization_mask operator|(unicode_normalization_mask const& lhs, unicode_normalization_mask const& rhs) noexcept
{
    return static_cast<unicode_normalization_mask>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr unicode_normalization_mask operator&(unicode_normalization_mask const& lhs, unicode_normalization_mask const& rhs) noexcept
{
    return static_cast<unicode_normalization_mask>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr bool operator==(unicode_decomposition_type const& lhs, unicode_normalization_mask const& rhs) noexcept
{
    return static_cast<bool>((1 << to_underlying(lhs)) & to_underlying(rhs));
}

}
