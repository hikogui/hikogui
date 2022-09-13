// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

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

constexpr long long pow10ll(int x) noexcept
{
    hi_axiom(x >= 0 && x <= 18);
    return pow10_table[x];
}

template<typename Iterator>
auto mean(Iterator first, Iterator last)
{
    hilet init = static_cast<typename std::iterator_traits<Iterator>::value_type>(0);

    hilet sum = std::reduce(first, last, init);
    hilet count = static_cast<decltype(sum)>(std::distance(first, last));

    return count > 0.0 ? sum / count : sum;
}

template<typename Iterator, typename T>
auto stddev(Iterator first, Iterator last, T mean)
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
constexpr void inplace_max(T& a, T const& b) noexcept
{
    a = std::max(a, b);
}

template<typename T>
constexpr void inplace_min(T& a, T const& b) noexcept
{
    a = std::min(a, b);
}

template<typename T>
constexpr void inplace_clamp(T& a, T const& lo, T const& hi) noexcept
{
    hi_axiom(lo <= hi);
    a = std::clamp(a, lo, hi);
}

template<typename T>
[[nodiscard]] constexpr T abs(T a) noexcept
{
    return a < T{} ? -a : a;
}

template<std::floating_point T>
[[nodiscard]] constexpr bool almost_equal(T a, T b) noexcept
{
    auto e = (a + b) * std::numeric_limits<T>::epsilon();
    return std::abs(a - b) <= e;
}

} // namespace hi::inline v1
