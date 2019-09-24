// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"

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

/*! Shift logical left with carry chain.
 * \param carry_in The bits received from a previous shl().
 * \param count number of bits to shift, must be less than the number of bits in T.
 * \param carry_out The bits that have been shifted out.
 */
template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr std::pair<T, T> shift_left_carry(T a, unsigned int count, T carry = 0) noexcept
{
    constexpr unsigned int nr_bits = sizeof(T) * 8;
    unsigned int reverse_count = nr_bits - count;

    return {
        (a << count)| carry,
        a >> reverse_count
    };
}

/*! Shift logical right with carry chain.
* \param carry_in The bits received from a previous shr().
* \param count number of bits to shift, must be less than the number of bits in T.
* \param carry_out The bits that have been shifted out.
*/
template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>, int> = 0>
constexpr std::pair<T, T> shift_right_carry(T a, unsigned int count, T carry = 0) noexcept
{
    constexpr unsigned int nr_bits = sizeof(T) * 8;
    unsigned int reverse_count = nr_bits - count;

    return {
        (a >> count)| carry,
        a << reverse_count
    };
}

/*! Subtract two numbers with borrow chain.
 * \return a - (b + borrow_in)
 */
template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr std::pair<T, T> subtract_borrow(T a, T b, T borrow = 0) noexcept
{
    if constexpr (sizeof(T) == 1) {
        uint16_t r = static_cast<uint16_t>(a) - (static_cast<uint16_t>(b) + borrow);
        return { static_cast<uint8_t>(r), static_cast<uint8_t>(r >> 8) };

    } else if constexpr (sizeof(T) == 2) {
        uint32_t r = static_cast<uint32_t>(a) - (static_cast<uint32_t>(b) + borrow);
        return { static_cast<uint16_t>(r), static_cast<uint16_t>(r >> 16) };

    } else if constexpr (sizeof(T) == 4) {
        uint64_t r = static_cast<uint64_t>(a) - (static_cast<uint64_t>(b) + borrow);
        return { static_cast<uint32_t>(r), static_cast<uint32_t>(r >> 32) };

    } else if constexpr (sizeof(T) == 8) {
#if PROCESSOR == CPU_X64
        uint64_t r;
        auto c = _subborrow_u64(static_cast<unsigned char>(borrow), a, b, &r);
        return { r, static_cast<uint64_t>(c) };
#elif COMPILER == CC_CLANG || COMPILER == CC_GCC
        uint128_t r = static_cast<uint128_t>(a) - (static_cast<uint128_t>(b) + borrow);
        return { static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64) };
#else
#error "Not implemented"
#endif
    }
}

/*! Add two numbers with carry chain.
* \return a + b + carry_in
*/
template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr std::pair<T, T> add_carry(T a, T b, T carry = 0) noexcept
{
    if constexpr (sizeof(T) == 1) {
        uint16_t r = static_cast<uint16_t>(a) + static_cast<uint16_t>(b) + carry;
        return { static_cast<uint8_t>(r), static_cast<uint8_t>(r >> 8) };

    } else if constexpr (sizeof(T) == 2) {
        uint32_t r = static_cast<uint32_t>(a) + static_cast<uint32_t>(b) + carry;
        return { static_cast<uint16_t>(r), static_cast<uint16_t>(r >> 16) };

    } else if constexpr (sizeof(T) == 4) {
        uint64_t r = static_cast<uint64_t>(a) + static_cast<uint64_t>(b) + carry;
        return { static_cast<uint32_t>(r), static_cast<uint32_t>(r >> 32) };

    } else if constexpr (sizeof(T) == 8) {
#if PROCESSOR == CPU_X64
        uint64_t r;
        auto c = _addcarryx_u64(static_cast<unsigned char>(carry), a, b, &r);
        return { r, static_cast<uint64_t>(c) };
#elif COMPILER == CC_CLANG || COMPILER == CC_GCC
        uint128_t r = static_cast<uint128_t>(a) + static_cast<uint128_t>(b) + carry;
        return { static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64) };
#else
#error "Not implemented"
#endif
    }
}

template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr std::pair<T, T> multiply_carry(T a, T b, T carry = 0, T accumulator = 0) noexcept
{
    if constexpr (sizeof(T) == 1) {
        uint16_t r = static_cast<uint16_t>(a) * static_cast<uint16_t>(b) + carry + accumulator;
        return { static_cast<uint8_t>(r), static_cast<uint8_t>(r >> 8) };

    } else if constexpr (sizeof(T) == 2) {
        uint32_t r = static_cast<uint32_t>(a) * static_cast<uint32_t>(b) + carry + accumulator;
        return { static_cast<uint16_t>(r), static_cast<uint16_t>(r >> 16) };

    } else if constexpr (sizeof(T) == 4) {
        uint64_t r = static_cast<uint64_t>(a) * static_cast<uint64_t>(b) + carry + accumulator;
        return { static_cast<uint32_t>(r), static_cast<uint32_t>(r >> 32) };

    } else if constexpr (sizeof(T) == 8) {
#if PROCESSOR == CPU_X64
        uint64_t hi;
        uint64_t lo = _mulx_u64(a, b, &hi);
        uint64_t c;
        std::tie(lo, c) = add_carry(lo, carry, uint64_t{0});
        std::tie(hi, c) = add_carry(hi, uint64_t{0}, c);
        std::tie(lo, c) = add_carry(lo, accumulator, uint64_t{0});
        std::tie(hi, c) = add_carry(hi, uint64_t{0}, c);
        return { lo, hi }; 

#elif COMPILER == CC_CLANG || COMPILER == CC_GCC
        uint128_t r = static_cast<uint128_t>(a) * static_cast<uint128_t>(b) + carry + accumulator;
        return { static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64) };
#else
#error "Not implemented"
#endif
    }
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
        return x << r | x >> (-r & mask);
    }
#else
    constexpr unsigned int mask = (8 * sizeof(T)) - 1;
    return x << r | x >> (-r & mask);
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
