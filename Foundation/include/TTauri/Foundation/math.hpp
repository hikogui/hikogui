// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/type_traits.hpp"
#include <complex>
#include <cmath>
#include <limits>
#include <gsl/gsl>
#include <tuple>
#include <numeric>
#include <iterator>

#if COMPILER == CC_MSVC
#include <intrin.h>
#endif
#if PROCESSOR == CPU_X64
#include <immintrin.h>
#endif

namespace TTauri {

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
    ttauri_assume(x >= 0 && x <= 18);
    return pow10_table[x];
}

template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr int bsr(T x) noexcept
{
#if COMPILER == CC_MSVC
    if constexpr (sizeof(T) == 4) {
        unsigned long index;
        auto found = _BitScanReverse(&index, x);
        return found ? index : -1;

    } else if constexpr (sizeof(T) == 8) {
        unsigned long index;
        auto found = _BitScanReverse64(&index, x);
        return found ? index : -1;

    } else {
        not_implemented;
    }
#elif COMPILER == CC_CLANG || COMPILER == CC_GCC
    if constexpr (std::is_same_v<T,unsigned int>) {
        auto tmp = __builtin_clz(x);
        return x == 0 ? -1 : ssizeof(T) * 8 - tmp - 1;

    } else if constexpr (std::is_same_v<T,unsigned long>) {
        auto tmp = __builtin_clzl(x);
        return x == 0 ? -1 : ssizeof(T) * 8 - tmp - 1;

    } else if constexpr (std::is_same_v<T,unsigned long long>) {
        auto tmp = __builtin_clzll(x);
        return x == 0 ? -1 : ssizeof(T) * 8 - tmp - 1;

    } else {
        not_implemented;
    }
#else
#error "Not implemented"
#endif
}

template<typename T>
constexpr auto next_power_of_two(T rhs)
{
    make_larger_t<T> x = rhs;

    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    if constexpr (sizeof(T) >= 2) {
        x |= x >> 8;
    }
    if constexpr (sizeof(T) >= 4) {
        x |= x >> 16;
    }
    if constexpr (sizeof(T) >= 8) {
        x |= x >> 32;
    }
    ++x;
    x += (x == 0);
    return x;
}

/** Make a bit-mask which includes the given value.
 */
template<typename T>
constexpr T make_mask(T x)
{
    let p2 = next_power_of_two(x);
    return static_cast<T>(p2 - 1);
}

/*! Bit scan reverse.
*
* \return index of highest '1' bit, or -1 when no bits are set.
*/
template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr int popcount(T x) noexcept
{
#if COMPILER == CC_MSVC
    if constexpr (sizeof(T) == sizeof(unsigned __int64)) {
        return __popcnt64(x);
    } else if constexpr (sizeof(T) == sizeof(unsigned int)) {
        return __popcnt(x);
    } else if constexpr (sizeof(T) == sizeof(unsigned short)) {
        return __popcnt16(x);
    } else {
        return __popcnt64(static_cast<unsigned __int64>(x));
    }
#elif COMPILER == CC_CLANG || COMPILER == CC_GCC
    if constexpr (std::is_same_v<T,unsigned int>) {
        return __builtin_popcount(x);

    } else if constexpr (std::is_same_v<T,unsigned long>) {
        return __builtin_popcountl(x);

    } else if constexpr (std::is_same_v<T,unsigned long long>) {
        return __builtin_popcountll(x);

    } else {
        not_implemented;
    }
#else
#error "Not implemented"
#endif
}

template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr T rotl(T x, unsigned int count) noexcept
{
#if COMPILER == CC_MSVC
    if constexpr (sizeof(T) == 1) {
        return _rotl8(x, count);
    } else if constexpr (sizeof(T) == 2) {
        return _rotl16(x, count);
    } else if constexpr (sizeof(T) == 4) {
        return _rotl(x, count);
    } else if constexpr (sizeof(T) == 8) {
        return _rotl64(x, count);
    } else {
        constexpr unsigned int mask = (8 * sizeof(T)) - 1;
        return x << count | x >> (-count & mask);
    }
#else
    constexpr unsigned int mask = (8 * sizeof(T)) - 1;
    return x << count | x >> (-count & mask);
#endif
}

template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr T rotr(T x, unsigned int count) noexcept
{
#if COMPILER == CC_MSVC
    if constexpr (sizeof(T) == 1) {
        return _rotr8(x, count);
    } else if constexpr (sizeof(T) == 2) {
        return _rotr16(x, count);
    } else if constexpr (sizeof(T) == 4) {
        return _rotr(x, count);
    } else if constexpr (sizeof(T) == 8) {
        return _rotr64(x, count);
    } else {
        constexpr unsigned int mask = (8 * sizeof(T)) - 1;
        return x >> count | x << (-count & mask);
    }
#else
    constexpr unsigned int mask = (8 * sizeof(T)) - 1;
    return x >> count | x << (-count & mask);
#endif
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
    let init = static_cast<typename std::iterator_traits<Iterator>::value_type>(0);

    let sum = std::reduce(first, last, init);
    let count = static_cast<decltype(sum)>(std::distance(first, last));
 
    return count > 0.0 ? sum / count : sum;
}

template<typename Iterator, typename T>
auto stddev(Iterator first, Iterator last, T mean)
{
    let init = static_cast<typename std::iterator_traits<Iterator>::value_type>(0);

    let sum = std::accumulate(first, last, init, [=](let &acc, let &value) {
        let tmp = value - mean;
        return acc + tmp*tmp;
    });

    let count = static_cast<decltype(sum)>(std::distance(first, last));
    return count > 0.0 ? sum / count : sum;
}

}
