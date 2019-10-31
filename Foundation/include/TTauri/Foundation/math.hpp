// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include <complex>
#include <cmath>
#include <limits>
#include <glm/glm.hpp>
#include <gsl/gsl>
#include <tuple>

#if COMPILER == CC_MSVC
#include <intrin.h>
#endif
#if PROCESSOR == CPU_X64
#include <immintrin.h>
#endif

namespace TTauri {

constexpr long double pi = 3.141592653589793238462643383279502884L;

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
        return x == 0 ? -1 : to_signed(sizeof(T)) * 8 - tmp - 1;

    } else if constexpr (std::is_same_v<T,unsigned long>) {
        auto tmp = __builtin_clzl(x);
        return x == 0 ? -1 : to_signed(sizeof(T)) * 8 - tmp - 1;

    } else if constexpr (std::is_same_v<T,unsigned long long>) {
        auto tmp = __builtin_clzll(x);
        return x == 0 ? -1 : to_signed(sizeof(T)) * 8 - tmp - 1;

    } else {
        not_implemented;
    }
#else
#error "Not implemented"
#endif
}

/*! Bit scan reverse.
*
* \return index of highest '1' bit, or -1 when no bits are set.
*/
template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr int popcount(T x) noexcept
{
#if COMPILER == CC_MSVC
    if constexpr (sizeof(T) == 8) {
        return PopulationCount64(x);
    } else {
        not_implemented;
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

template<typename T, typename M>
constexpr T modulo(T x, M m) noexcept
{
    if (x >= 0) {
        return x % m;
    } else {
        return m - (-x % m);
    }
}

}
