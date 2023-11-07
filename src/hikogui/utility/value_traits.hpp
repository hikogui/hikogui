

#pragma once

#include "../macros.hpp"
#include <concepts>

/** @file utility/value_traits.hpp Utility functions to determine information about values.
 * @ingroup utility
 */
hi_export_module(hikogui.utility.value_traits);

hi_export namespace hi { inline namespace v1 {

/** Check if a value is integral.
 *
 * @ingroup utility
 * @param rhs A integral value.
 * @return Always true.
 */
template<std::integral T>
[[nodiscard]] constexpr bool is_integral_value(T const &rhs) noexcept
{
    return true;
}

/** Check if a value is integral.
 *
 * @ingroup utility
 * @param rhs A floating point value.
 * @return True if the floating point value is an integer.
 */
template<std::floating_point T>
[[nodiscard]] constexpr bool is_integral_value(T const &rhs) noexcept
{
    return static_cast<double>(static_cast<long long>(rhs)) == static_cast<double>(rhs);
}

}}

