// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <type_traits>
#include <limits>

namespace tt {

template<typename To, typename From>
struct is_lossless_cast {
    static_assert(std::is_floating_point_v<To> || std::is_integral_v<To>, "is_lossless_cast 'To' must be float or integer");
    static_assert(std::is_floating_point_v<From> || std::is_integral_v<From>, "is_lossless_cast 'From' must be float or integer");

    static constexpr bool value = 
        std::is_floating_point_v<To> ||
        (std::is_signed_v<To> && sizeof(To) > sizeof(From)) ||
        (std::is_signed_v<To> == std::is_signed_v<To> && sizeof(To) >= sizeof(From));
};

template<typename To, typename From>
constexpr bool is_lossless_cast_v = is_lossless_cast<To,From>::value;


/*! Convert numeric values to other type with checks.
 * When conversion fails std::terminate() is called.
 */
template<typename To, typename From>
constexpr To numeric_cast(From x) noexcept
{
    if constexpr (!is_lossless_cast_v<To,From>) {
        if constexpr (std::is_signed_v<To> == std::is_signed_v<From>) {
            // When both sides have the same signess, then they both get converted to
            // the largest fitting type.
            if constexpr (std::is_signed_v<To>) {
                // Only signed numbers can be less than zero.
                tt_assume(x >= std::numeric_limits<To>::min());
            }
            tt_assume(x <= std::numeric_limits<To>::max());

        } else if constexpr (std::is_unsigned_v<To>) {
            // When the destination is unsigned and source is signed.
            tt_assume(x >= 0);
            if constexpr(sizeof(To) < sizeof(From)) {
                // When signed may not fit inside the unsigned number check by first converting
                // the maximum destination to the larger signed type.
                tt_assume(x <= static_cast<From>(std::numeric_limits<To>::max()));
            }

        } else {
            // When the destination is signed and source is unsigned.
            if constexpr(sizeof(To) <= sizeof(From)) {
                // When unsigned may not fit inside the signed number check by first converting
                // the maximum destination to the larger-or-equal unsigned type.
                tt_assume(x <= static_cast<From>(std::numeric_limits<To>::max()));
            }
        }
    }
    return static_cast<To>(x);
}

}
