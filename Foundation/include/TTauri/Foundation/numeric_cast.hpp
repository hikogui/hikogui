// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <type_traits>
#include <limits>

namespace TTauri {

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
template<typename T, typename U>
constexpr T numeric_cast(U x) noexcept
{
    if constexpr (!is_lossless_cast_v<T,U>) {
        if constexpr (std::is_signed_v<U>) {
            ttauri_assume(x >= std::numeric_limits<T>::min());
        }
        ttauri_assume(x <= std::numeric_limits<T>::max());
    }
    return static_cast<T>(x);
}

/*! Fast conversion from a numeric value to a signed value of the same size.
 */
template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
constexpr auto to_signed(T x) noexcept
{
    return numeric_cast<std::make_signed_t<T>>(x);
}

/*! Fast conversion from a numeric value to a unsigned value of the same size.
*/
template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
constexpr auto to_unsigned(T x) noexcept
{
    return numeric_cast<std::make_unsigned_t<T>>(x);
}

}
