// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "cast.hpp"
#include <complex>
#include <cmath>
#include <limits>
#include <span>
#include <tuple>
#include <concepts>

#if TT_COMPILER == TT_CC_MSVC
#include <intrin.h>
#endif
#if TT_PROCESSOR == TT_CPU_X64
#include <immintrin.h>
#endif

namespace tt {

/** Shift logical left with carry chain.
 * @param lhs The original value
 * @param rhs The count by how much to shift lhs left.
 * @param carry The carry data to or with the lower bits.
 * @return The result, followed by the carry which can be used to pass into the next iteration.
 */
template<std::unsigned_integral T>
constexpr std::pair<T, T> shift_left_carry(T lhs, unsigned int rhs, T carry = 0) noexcept
{
    constexpr unsigned int nr_bits = sizeof(T) * 8;
    unsigned int reverse_count = nr_bits - rhs;

    return {(lhs << rhs) | carry, lhs >> reverse_count};
}

/** Shift logical right with carry chain.
 * @param lhs The original value
 * @param rhs The count by how much to shift lhs right.
 * @param carry The carry data to or with the lower bits.
 * @return The result, followed by the carry which can be used to pass into the next iteration.
 */
template<std::unsigned_integral T>
constexpr std::pair<T, T> shift_right_carry(T lhs, unsigned int rhs, T carry = 0) noexcept
{
    constexpr unsigned int nr_bits = sizeof(T) * 8;
    unsigned int reverse_count = nr_bits - rhs;

    return {(lhs >> rhs) | carry, lhs << reverse_count};
}

/*! Add two numbers with carry chain.
 * \param carry either 0 or 1.
 * \return a + b + carry_in
 */
template<std::unsigned_integral T>
constexpr std::pair<T, T> add_carry(T a, T b, T carry = 0) noexcept
{
    if constexpr (sizeof(T) == 1) {
        uint16_t r = static_cast<uint16_t>(a) + static_cast<uint16_t>(b) + carry;
        return {static_cast<uint8_t>(r), static_cast<uint8_t>(r >> 8)};

    } else if constexpr (sizeof(T) == 2) {
        uint32_t r = static_cast<uint32_t>(a) + static_cast<uint32_t>(b) + carry;
        return {static_cast<uint16_t>(r), static_cast<uint16_t>(r >> 16)};

    } else if constexpr (sizeof(T) == 4) {
        uint64_t r = static_cast<uint64_t>(a) + static_cast<uint64_t>(b) + carry;
        return {static_cast<uint32_t>(r), static_cast<uint32_t>(r >> 32)};

    } else if constexpr (sizeof(T) == 8) {
#if TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        auto r = static_cast<__uint128_t>(a) + static_cast<__uint128_t>(b) + carry;
        return {static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64)};
#else
        uint64_t r1 = a + b;
        uint64_t c = (r1 < a) ? 1 : 0;
        uint64_t r2 = r1 + carry;
        c += (r2 < r1) ? 1 : 0;
        return {r2, c};
#endif
    }
}

template<std::unsigned_integral T>
constexpr std::pair<T, T> wide_mul(T a, T b) noexcept
{
    if constexpr (sizeof(T) == 1) {
        uint16_t r = static_cast<uint16_t>(a) * static_cast<uint16_t>(b);
        return {static_cast<uint8_t>(r), static_cast<uint8_t>(r >> 8)};

    } else if constexpr (sizeof(T) == 2) {
        uint32_t r = static_cast<uint32_t>(a) * static_cast<uint32_t>(b);
        return {static_cast<uint16_t>(r), static_cast<uint16_t>(r >> 16)};

    } else if constexpr (sizeof(T) == 4) {
        uint64_t r = static_cast<uint64_t>(a) * static_cast<uint64_t>(b);
        return {static_cast<uint32_t>(r), static_cast<uint32_t>(r >> 32)};

    } else if constexpr (sizeof(T) == 8) {
#if TT_COMPILER == TT_CC_MSVC
        uint64_t hi = 0;
        uint64_t lo = _umul128(a, b, &hi);
        return {lo, hi};

#elif TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        auto r = static_cast<__uint128_t>(a) * static_cast<__uint128_t>(b);
        return {static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64)};
#else
#error "Not implemented"
#endif
    }
}

template<std::unsigned_integral T>
constexpr std::pair<T, T> mul_carry(T a, T b, T carry = 0, T accumulator = 0) noexcept
{
    if constexpr (sizeof(T) == 1) {
        uint16_t r = static_cast<uint16_t>(a) * static_cast<uint16_t>(b) + carry + accumulator;
        return {static_cast<uint8_t>(r), static_cast<uint8_t>(r >> 8)};

    } else if constexpr (sizeof(T) == 2) {
        uint32_t r = static_cast<uint32_t>(a) * static_cast<uint32_t>(b) + carry + accumulator;
        return {static_cast<uint16_t>(r), static_cast<uint16_t>(r >> 16)};

    } else if constexpr (sizeof(T) == 4) {
        uint64_t r = static_cast<uint64_t>(a) * static_cast<uint64_t>(b) + carry + accumulator;
        return {static_cast<uint32_t>(r), static_cast<uint32_t>(r >> 32)};

    } else if constexpr (sizeof(T) == 8) {
#if TT_COMPILER == TT_CC_MSVC
        uint64_t hi = 0;
        uint64_t lo = _umul128(a, b, &hi);
        uint64_t c = 0;
        std::tie(lo, c) = add_carry(lo, carry, uint64_t{0});
        std::tie(hi, c) = add_carry(hi, uint64_t{0}, c);
        std::tie(lo, c) = add_carry(lo, accumulator, uint64_t{0});
        std::tie(hi, c) = add_carry(hi, uint64_t{0}, c);
        return {lo, hi};

#elif TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        auto r = static_cast<__uint128_t>(a) * static_cast<__uint128_t>(b) + carry + accumulator;
        return {static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64)};
#else
#error "Not implemented"
#endif
    }
}

template<std::unsigned_integral T>
constexpr T wide_div(T lhs_lo, T lhs_hi, T rhs) noexcept
{
    if constexpr (sizeof(T) == 1) {
        ttlet lhs = static_cast<uint16_t>(lhs_hi) << 8 | static_cast<uint16_t>(lhs_lo);
        return narrow_cast<uint8_t>(lhs / rhs);

    } else if constexpr (sizeof(T) == 2) {
        ttlet lhs = static_cast<uint32_t>(lhs_hi) << 16 | static_cast<uint32_t>(lhs_lo);
        return narrow_cast<uint16_t>(lhs / rhs);

    } else if constexpr (sizeof(T) == 4) {
        ttlet lhs = static_cast<uint64_t>(lhs_hi) << 32 | static_cast<uint64_t>(lhs_lo);
        return narrow_cast<uint32_t>(lhs / rhs);

    } else if constexpr (sizeof(T) == 8) {
#if TT_COMPILER == TT_CC_MSVC
        uint64_t remainder;
        return _udiv128(lhs_hi, lhs_lo, rhs, &remainder);

#elif TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        ttlet lhs = static_cast<__uint128_t>(lhs_hi) << 64 | static_cast<__uint128_t>(lhs_lo);
        return narrow_cast<uint64_t>(lhs / rhs);
#else
#error "Not implemented"
#endif
    }
}

} // namespace tt
