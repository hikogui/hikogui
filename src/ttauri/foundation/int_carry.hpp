// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/required.hpp"
#include "ttauri/foundation/numeric_cast.hpp"
#include <complex>
#include <cmath>
#include <limits>
#include <nonstd/span>
#include <tuple>

#if TT_COMPILER == TT_CC_MSVC
#include <intrin.h>
#endif
#if TT_PROCESSOR == TT_CPU_X64
#include <immintrin.h>
#endif

namespace tt {

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
        (a << count) | carry,
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
        (a >> count) | carry,
        a << reverse_count
    };
}


/*! Add two numbers with carry chain.
* \param carry either 0 or 1.
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
#if TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        auto r = static_cast<__uint128_t>(a) + static_cast<__uint128_t>(b) + carry;
        return { static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64) };
#else
        uint64_t r1 = a + b;
        uint64_t c = (r1 < a) ? 1 : 0;
        uint64_t r2 = r1 + carry;
        c += (r2 < r1) ? 1 : 0;
        return { r2, c };
#endif
    }
}

template<typename T, std::enable_if_t<std::is_integral_v<T> && std::is_unsigned_v<T>,int> = 0>
constexpr std::pair<T, T> wide_multiply(T a, T b) noexcept
{
    if constexpr (sizeof(T) == 1) {
        uint16_t r = static_cast<uint16_t>(a) * static_cast<uint16_t>(b);
        return { static_cast<uint8_t>(r), static_cast<uint8_t>(r >> 8) };

    } else if constexpr (sizeof(T) == 2) {
        uint32_t r = static_cast<uint32_t>(a) * static_cast<uint32_t>(b);
        return { static_cast<uint16_t>(r), static_cast<uint16_t>(r >> 16) };

    } else if constexpr (sizeof(T) == 4) {
        uint64_t r = static_cast<uint64_t>(a) * static_cast<uint64_t>(b);
        return { static_cast<uint32_t>(r), static_cast<uint32_t>(r >> 32) };

    } else if constexpr (sizeof(T) == 8) {
#if TT_COMPILER == TT_CC_MSVC
        uint64_t hi = 0;
        uint64_t lo = _umul128(a, b, &hi);
        return { lo, hi }; 

#elif TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        auto r = static_cast<__uint128_t>(a) * static_cast<__uint128_t>(b);
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
#if TT_COMPILER == TT_CC_MSVC
        uint64_t hi = 0;
        uint64_t lo = _umul128(a, b, &hi);
        uint64_t c = 0;
        std::tie(lo, c) = add_carry(lo, carry, uint64_t{0});
        std::tie(hi, c) = add_carry(hi, uint64_t{0}, c);
        std::tie(lo, c) = add_carry(lo, accumulator, uint64_t{0});
        std::tie(hi, c) = add_carry(hi, uint64_t{0}, c);
        return { lo, hi }; 

#elif TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        auto r = static_cast<__uint128_t>(a) * static_cast<__uint128_t>(b) + carry + accumulator;
        return { static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64) };
#else
#error "Not implemented"
#endif
    }
}

}
