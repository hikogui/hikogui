// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"

#include <complex>
#include <cmath>
#include <limits>
#include <glm/glm.hpp>
#include <gsl/gsl>

#if COMPILER == CC_MSVC
#include <intrin.h>
#endif

namespace TTauri {

constexpr long double pi = 3.141592653589793238462643383279502884L;

template<typename T>
inline std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
rotl(T x, unsigned int count)
{
#if COMPILER == CC_MSVC
    static_assert(sizeof(T) <= 4);
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
        return x << r | x >> (-r & mask);
    }
#else
    constexpr unsigned int mask = (8 * sizeof(T)) - 1;
    return x << r | x >> (-r & mask);
#endif
}

template<typename T>
inline std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, T>
rotr(T x, unsigned int count)
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
        return x >> r | x << (-r & mask);
    }
#else
    constexpr unsigned int mask = (8 * sizeof(T)) - 1;
    return x >> r | x << (-r & mask);
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
