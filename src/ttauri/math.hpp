// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "numeric_cast.hpp"
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

constexpr long double pi = 3.141592653589793238462643383279502884L;

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
    tt_assume(x >= 0 && x <= 18);
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

inline bool almost_equal(float a, float b) noexcept {
    uint32_t a_;
    uint32_t b_;

    std::memcpy(&a_, &a, sizeof(a_));
    std::memcpy(&b_, &b, sizeof(b_));

    auto a__ = static_cast<int>(a_ & 0x7ffffff);
    auto b__ = static_cast<int>(b_ & 0x7ffffff);

    if ((a_ < 0) == (b_ < 0)) {
        return std::abs(a__ - b__) < 10;
    } else {
        return std::abs(a__ + b__) < 10;
    }
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
