// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "cast.hpp"
#include "type_traits.hpp"
#include <complex>
#include <cmath>
#include <limits>
#include <span>
#include <tuple>
#include <numeric>
#include <iterator>
#include <bit>

#if TT_COMPILER == TT_CC_MSVC
#include <intrin.h>
#endif
#if TT_PROCESSOR == TT_CPU_X64
#include <immintrin.h>
#endif

namespace tt {

constexpr long long pow10_table[20] {
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

constexpr long long pow10ll(int x) noexcept {
    tt_axiom(x >= 0 && x <= 18);
    return pow10_table[x];
}

/** Find the position of the left-most '1' bit.
 * @return Bit position of the left-most '1' bit, or -1 if there is no '1' bit.
 */
template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr int bsr(T x) noexcept
{
    return static_cast<int>(sizeof(T) * 8 - std::countr_zero(x)) - 1;
}

/** Make a bit-mask which includes the given value.
 */
template<typename T>
constexpr T make_mask(T x)
{
    ttlet x_ = static_cast<unsigned long long>(x) + 1;
    ttlet p2 = std::bit_ceil(x_);
    return static_cast<T>(p2 - 1);
}


template<typename T>
constexpr T median(T a, T b, T c) noexcept {
    return std::clamp(c, std::min(a, b), std::max(a, b));
}

/** Compare two floating point values to be almost equal.
 * The two floating point values are almost equal if they are
 * at most 10 smallest float steps away from each other.
 */
constexpr bool almost_equal(float a, float b) noexcept {
    constexpr int32_t epsilon = 5;

    ttlet a_ = std::bit_cast<int32_t>(a);
    ttlet b_ = std::bit_cast<int32_t>(b);

    // Strip the sign bit, and extend to not overflow when calculating the delta.
    ttlet a_abs = static_cast<int64_t>(a_ & 0x7ffffff);
    ttlet b_abs = static_cast<int64_t>(b_ & 0x7ffffff);

    // If both floats have the same size, we can subtract to get the delta.
    // If they have a different sign then we need to add.
    ttlet delta = (a_ < 0) == (b_ < 0) ? a_abs - b_abs : a_abs + b_abs;

    // Check if the delta is with epsilon.
    return delta < epsilon && delta > -epsilon;
}

template<typename Iterator>
auto mean(Iterator first, Iterator last)
{
    ttlet init = static_cast<typename std::iterator_traits<Iterator>::value_type>(0);

    ttlet sum = std::reduce(first, last, init);
    ttlet count = static_cast<decltype(sum)>(std::distance(first, last));
 
    return count > 0.0 ? sum / count : sum;
}

template<typename Iterator, typename T>
auto stddev(Iterator first, Iterator last, T mean)
{
    ttlet init = static_cast<typename std::iterator_traits<Iterator>::value_type>(0);

    ttlet sum = std::accumulate(first, last, init, [=](ttlet &acc, ttlet &value) {
        ttlet tmp = value - mean;
        return acc + tmp*tmp;
    });

    ttlet count = static_cast<decltype(sum)>(std::distance(first, last));
    return count > 0.0 ? sum / count : sum;
}

}
