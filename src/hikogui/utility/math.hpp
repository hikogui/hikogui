// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file math.hpp Miscellaneous math functions.
 */

#pragma once

#include "utility.hpp"
#include "cast.hpp"
#include "type_traits.hpp"
#include "assert.hpp"
#include <complex>
#include <cmath>
#include <limits>
#include <span>
#include <tuple>
#include <numeric>
#include <iterator>
#include <bit>
#include <concepts>
#include <algorithm>
#include <numbers>

#if HI_COMPILER == HI_CC_MSVC
#include <intrin.h>
#endif
#if HI_PROCESSOR == HI_CPU_X64
#include <immintrin.h>
#endif

namespace hi::inline v1 {

constexpr long long pow10_table[20]{
    1LL,
    10LL,
    100LL,
    1'000LL,
    10'000LL,
    100'000LL,
    1'000'000LL,
    10'000'000LL,
    100'000'000LL,
    1'000'000'000LL,
    10'000'000'000LL,
    100'000'000'000LL,
    1'000'000'000'000LL,
    10'000'000'000'000LL,
    100'000'000'000'000LL,
    1'000'000'000'000'000LL,
    10'000'000'000'000'000LL,
    100'000'000'000'000'000LL,
    1'000'000'000'000'000'000LL,
};

[[nodiscard]] constexpr long long pow10ll(int x) noexcept
{
    hi_axiom(x >= 0 && x <= 18);
    return pow10_table[x];
}

template<typename Iterator>
[[nodiscard]] auto mean(Iterator first, Iterator last)
{
    hilet init = static_cast<typename std::iterator_traits<Iterator>::value_type>(0);

    hilet sum = std::reduce(first, last, init);
    hilet count = static_cast<decltype(sum)>(std::distance(first, last));

    return count > 0.0 ? sum / count : sum;
}

template<typename Iterator, typename T>
[[nodiscard]] auto stddev(Iterator first, Iterator last, T mean)
{
    hilet init = static_cast<typename std::iterator_traits<Iterator>::value_type>(0);

    hilet sum = std::accumulate(first, last, init, [=](hilet& acc, hilet& value) {
        hilet tmp = value - mean;
        return acc + tmp * tmp;
    });

    hilet count = static_cast<decltype(sum)>(std::distance(first, last));
    return count > 0.0 ? sum / count : sum;
}

template<typename T>
constexpr bool inplace_max(T& a, T const& b) noexcept
{
    using std::max;
    a = max(a, b);
    return a == b;
}

template<typename T>
constexpr bool inplace_min(T& a, T const& b) noexcept
{
    using std::min;
    a = min(a, b);
    return a == b;
}

template<typename T>
constexpr void inplace_clamp(T& a, T const& lo, T const& hi) noexcept
{
    hi_axiom(lo <= hi);

    using std::clamp;
    a = clamp(a, lo, hi);
}


/** Absolute value of a signed number converted to an unsigned number.
 *
 * This function correctly handles `std::numeric_limits<T>::min()`.
 *
 * @param rhs A signed number.
 * @return An unsigned number.
 */
template<std::signed_integral T>
[[nodiscard]] constexpr friend std::make_unsigned_t<T> abs_unsigned(T rhs) noexcept
{
    auto rhs_u = static_cast<std::make_unsigned_t<T>>(rhs);
    auto res_u = -rhs_u;

    // All positive numbers and int_min, would result in a negative number.
    if (static_cast<T>(res_u) < 0) {
        // int_min casted to unsigned in int_max + 1.
        res_u = rhs_u;
    }
    return rhs_u;
}

template<std::floating_point T>
[[nodiscard]] constexpr bool almost_equal(T a, T b) noexcept
{
    auto e = (a + b) * std::numeric_limits<T>::epsilon();
    return std::abs(a - b) <= e;
}

/** Convert degree to radian.
 *
 * @param degree The number of degrees.
 * @return The number of radians.
 */
template<std::floating_point T>
[[nodiscard]] constexpr T to_radian(T degree) noexcept
{
    return degree * (std::numbers::pi_v<T> / T{180.0});
}

/** The greatest multiple of alignment less than or equal to value.
 * @param value The unsigned value to round.
 * @param alignment The alignment.
 * @return The greatest multiple of alignment less than or equal to value.
 */
template<std::unsigned_integral T>
[[nodiscard]] constexpr T floor(T value, T alignment) noexcept
{
    return (value / alignment) * alignment;
}

/** The smallest multiple of alignment greater than or equal to value.
 * @param value The unsigned value to round.
 * @param alignment The alignment.
 * @return The smallest multiple of alignment greater than or equal to value.
 */
template<std::unsigned_integral T>
[[nodiscard]] constexpr T ceil(T value, T alignment) noexcept
{
    return floor(value + (alignment - 1), alignment);
}

template<std::floating_point T>
[[nodiscard]] constexpr bool isnan(T value) noexcept
{
    return not (value == value);
}

} // namespace hi::inline v1
