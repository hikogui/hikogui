// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include <type_traits>
#include <limits>

namespace TTauri {

/*! Fast conversion from a numeric value to a signed value of the same size.
 * Unsigned integer values that are too large are mapped to negative values.
 */
template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
constexpr auto to_signed(T x) noexcept
{
    return static_cast<std::make_signed_t<T>>(x);
}

/*! Fast conversion from a numeric value to a unsigned value of the same size.
* Negative signed integer values that are less than zero are mapped to large positive values.
*/
template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
constexpr auto to_unsigned(T x) noexcept
{
    return static_cast<std::make_unsigned_t<T>>(x);
}

/*! Convert numeric values to other type with checks.
 * When conversion fails std::terminate() is called.
 */
template<typename T, typename U>
constexpr T numeric_cast(U x) noexcept
{
    if constexpr (std::is_floating_point_v<T>) {
        // No check needed all signed and unsigned integers can be cast without overflow.

    } else if constexpr (std::is_signed_v<T> && std::is_signed_v<U> && (sizeof(T) < sizeof(U))) {
        axiom_assert(x >= std::numeric_limits<T>::min() && x <= std::numeric_limits<T>::max());

    } else if constexpr (std::is_signed_v<T> && std::is_unsigned_v<U> && (sizeof(T) <= sizeof(U))) {
        axiom_assert(x <= static_cast<U>(std::numeric_limits<T>::max()));

    } else if constexpr (std::is_unsigned_v<T> && std::is_unsigned_v<U> && (sizeof(T) < sizeof(U))) {
        axiom_assert(x <= std::numeric_limits<T>::max());

    } else if constexpr (std::is_unsigned_v<T> && std::is_signed_v<U>) {
        if constexpr (sizeof(T) < sizeof(U)) {
            axiom_assert(x >= 0 && x <= std::numeric_limits<T>::max());
        } else {
            axiom_assert(x >= 0);
        }
    }
    return static_cast<T>(x);
}

}
