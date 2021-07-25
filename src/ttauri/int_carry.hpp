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
    constexpr unsigned int num_bits = sizeof(T) * 8;
    unsigned int reverse_count = num_bits - rhs;

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
    constexpr unsigned int num_bits = sizeof(T) * 8;
    unsigned int reverse_count = num_bits - rhs;

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
    tt_axiom(carry <= 1);

    constexpr size_t num_bits = sizeof(T) * CHAR_BIT;

    if constexpr (has_uintxx_v<num_bits * 2>) {
        // We can use a native type that has double the size.
        using U = make_uintxx_t<num_bits * 2>;

        ttlet r = static_cast<U>(lhs) + static_cast<U>(rhs) + static_cast<U>(carry);
        return {static_cast<T>(r), static_cast<T>(r >> num_bits)};

    } else if (not std::is_constant_evaluated()) {
#if TT_COMPILER == TT_MSVC
        uint64_t r;
        ttlet c = _addcarry_u64(static_cast<unsigned char>(carry), lhs, rhs, &r);
        return {r, static_cast<T>(c)};
#endif
    }

    // Carry can directly be added the sum without a double overflow.
    ttlet r = lhs + rhs + carry;
    ttlet c = static_cast<T>(r1 < lhs);
    return {r, c};
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
 * @return low, high of the result.
 */
template<std::unsigned_integral T>
constexpr std::pair<T, T> mul_carry(T lhs, T rhs, T carry = 0, T accumulator = 0) noexcept
{
    constexpr size_t num_bits = sizeof(T) * CHAR_BIT;

    if constexpr (has_uintxx_v<num_bits * 2>) {
        // We can use a native type that has double the size.
        using U = make_uintxx_t<num_bits * 2>;

        ttlet r = static_cast<U>(lhs) * static_cast<U>(rhs) + static_cast<U>(carry) + static_cast<U>(accumulator);
        return {static_cast<T>(r), static_cast<T>(r >> num_bits)};

    } else if (not std::is_constant_evaluated()) {
#if TT_COMPILER == TT_CC_MSVC
        if constexpr (sizeof(T) == 8)
            uint64_t hi = 0;
            uint64_t lo = _umul128(lhs, rhs, &hi);
            uint64_t c = 0;
            std::tie(lo, c) = add_carry(lo, carry, uint64_t{0});
            std::tie(hi, c) = add_carry(hi, uint64_t{0}, c);
            std::tie(lo, c) = add_carry(lo, accumulator, uint64_t{0});
            std::tie(hi, c) = add_carry(hi, uint64_t{0}, c);
            return {lo, hi};
        }
#endif
    }

    constexpr size_t num_half_bits = num_bits / 2;
    constexpr T half_mask = (T{1} << num_half_bits) - T{1};

    ttlet A = lhs >> num_half_bits;
    ttlet B = lhs & half_mask;
    ttlet C = rhs >> num_half_bits;
    ttlet D = rhs & half_mask;
    ttlet AC = A * C;
    ttlet AD = A * D;
    ttlet BC = B * C;
    ttlet BD = B * D;

    // Provisional result.
    auto hi = AC;
    auto lo = BD;
    auto c = T{0};

    // AD and BC are shifted half way accross the lo and hi of the result.
    ttlet AD_lo = AD << num_half_bits;
    ttlet AD_hi = AD >> num_half_bits;
    ttlet BC_lo = BC << num_half_bits;
    ttlet BC_hi = BC >> num_half_bits;

    std::tie(lo, c) = add_carry(lo, AD_lo, T{0});
    std::tie(hi, c) = add_carry(hi, AD_hi, c);
    std::tie(lo, c) = add_carry(lo, BC_lo, T{0});
    std::tie(hi, c) = add_carry(hi, BC_hi, c);

    // Now add the carry and accumulator arguments.
    std::tie(lo, c) = add_carry(lo, carry, T{0});
    std::tie(hi, c) = add_carry(hi, T{0}, c);
    std::tie(lo, c) = add_carry(lo, accumulator, T{0});
    std::tie(hi, c) = add_carry(hi, T{0}, c);
    return {lo, hi};
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
