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
 * @return (result, carry); the carry which can be used to pass into the next iteration.
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
 * @return (result, carry); the carry which can be used to pass into the next iteration.
 */
template<std::unsigned_integral T>
constexpr std::pair<T, T> shift_right_carry(T lhs, unsigned int rhs, T carry = 0) noexcept
{
    constexpr unsigned int nr_bits = sizeof(T) * 8;
    unsigned int reverse_count = nr_bits - rhs;

    return {(lhs >> rhs) | carry, lhs << reverse_count};
}

/** Add two numbers with carry chain.
 * @param lhs The left hand side
 * @param rhs The right hand side
 * @param carry From the previous add in the chain
 * @return (result, carry) pair
 */
template<std::unsigned_integral T>
constexpr std::pair<T, T> add_carry(T lhs, T rhs, T carry = 0) noexcept
{
    tt_axiom(carry == 0 || carry == 1);

    if constexpr (sizeof(T) == 1) {
        uint16_t r = static_cast<uint16_t>(lhs) + static_cast<uint16_t>(rhs) + carry;
        return {static_cast<uint8_t>(r), static_cast<uint8_t>(r >> 8)};

    } else if constexpr (sizeof(T) == 2) {
        uint32_t r = static_cast<uint32_t>(lhs) + static_cast<uint32_t>(rhs) + carry;
        return {static_cast<uint16_t>(r), static_cast<uint16_t>(r >> 16)};

    } else if constexpr (sizeof(T) == 4) {
        uint64_t r = static_cast<uint64_t>(lhs) + static_cast<uint64_t>(rhs) + carry;
        return {static_cast<uint32_t>(r), static_cast<uint32_t>(r >> 32)};

    } else if constexpr (sizeof(T) == 8) {
#if TT_COMPILER == TT_MSVC
        uint64_t r;
        auto carry_out = _addcarry_u64(static_cast<unsigned char>(carry), lhs, rhs, &r);
        return {r, static_cast<uint64_t>(carry_out)};
#elif TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        auto r = static_cast<__uint128_t>(lhs) + static_cast<__uint128_t>(rhs) + carry;
        return {static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64)};
#else
        uint64_t r1 = lhs + rhs;
        uint64_t c = (r1 < lhs) ? 1 : 0;
        uint64_t r2 = r1 + carry;
        c += (r2 < r1) ? 1 : 0;
        return {r2, c};
#endif
    }
}

/** Multiply with carry.
 * The carry is a high-word of the multiplication result and has the same size
 * as the inputs. The accumulator is used when doing long-multiplication from the
 * previous row. This function does not overflow even if all the arguments are at max.
 *
 * @param lhs The left hand side.
 * @param rhs The right hand side.
 * @param carry The carry-input; carry-output from the previous `mul_carry()`.
 * @param accumulator The column value during a long multiply.
 */
template<std::unsigned_integral T>
constexpr std::pair<T, T> mul_carry(T lhs, T rhs, T carry = 0, T accumulator = 0) noexcept
{
    if constexpr (sizeof(T) == 1) {
        uint16_t r = static_cast<uint16_t>(lhs) * static_cast<uint16_t>(rhs) + carry + accumulator;
        return {static_cast<uint8_t>(r), static_cast<uint8_t>(r >> 8)};

    } else if constexpr (sizeof(T) == 2) {
        uint32_t r = static_cast<uint32_t>(lhs) * static_cast<uint32_t>(rhs) + carry + accumulator;
        return {static_cast<uint16_t>(r), static_cast<uint16_t>(r >> 16)};

    } else if constexpr (sizeof(T) == 4) {
        uint64_t r = static_cast<uint64_t>(lhs) * static_cast<uint64_t>(rhs) + carry + accumulator;
        return {static_cast<uint32_t>(r), static_cast<uint32_t>(r >> 32)};

    } else if constexpr (sizeof(T) == 8) {
#if TT_COMPILER == TT_CC_MSVC
        uint64_t hi = 0;
        uint64_t lo = _umul128(lhs, rhs, &hi);
        uint64_t c = 0;
        std::tie(lo, c) = add_carry(lo, carry, uint64_t{0});
        std::tie(hi, c) = add_carry(hi, uint64_t{0}, c);
        std::tie(lo, c) = add_carry(lo, accumulator, uint64_t{0});
        std::tie(hi, c) = add_carry(hi, uint64_t{0}, c);
        return {lo, hi};

#elif TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        auto r = static_cast<__uint128_t>(lhs) * static_cast<__uint128_t>(rhs) + carry + accumulator;
        return {static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64)};
#else
#error "Not implemented"
#endif
    }
}

/** Wide multiply.
 * multiplies two numbers and returns a low, high pair.
 *
 * @param lhs The left hand side.
 * @param rhs The right hand side.
 * @return (low, high) result.
 */
template<std::unsigned_integral T>
constexpr std::pair<T, T> wide_mul(T lhs, T rhs) noexcept
{
    if constexpr (sizeof(T) == 1) {
        uint16_t r = static_cast<uint16_t>(lhs) * static_cast<uint16_t>(rhs);
        return {static_cast<uint8_t>(r), static_cast<uint8_t>(r >> 8)};

    } else if constexpr (sizeof(T) == 2) {
        uint32_t r = static_cast<uint32_t>(lhs) * static_cast<uint32_t>(rhs);
        return {static_cast<uint16_t>(r), static_cast<uint16_t>(r >> 16)};

    } else if constexpr (sizeof(T) == 4) {
        uint64_t r = static_cast<uint64_t>(lhs) * static_cast<uint64_t>(rhs);
        return {static_cast<uint32_t>(r), static_cast<uint32_t>(r >> 32)};

    } else if constexpr (sizeof(T) == 8) {
#if TT_COMPILER == TT_CC_MSVC
        uint64_t hi = 0;
        uint64_t lo = _umul128(lhs, rhs, &hi);
        return {lo, hi};

#elif TT_COMPILER == TT_CC_CLANG || TT_COMPILER == TT_CC_GCC
        auto r = static_cast<__uint128_t>(lhs) * static_cast<__uint128_t>(rhs);
        return {static_cast<uint64_t>(r), static_cast<uint64_t>(r >> 64)};
#else
#error "Not implemented"
#endif
    }
}

/** Wide divide.
 * Can be used to divide a wide unsigned integer by a unsigned integer,
 * as long as the result fits in an unsigned integer.
 * 
 * @param lhs_lo The low side of a wide left-hand-side
 * @param lhs_hi The high side of a wide left-hand-side
 * @param rhs The right hand side
 * @retrun The result.
 */
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
