// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "assert.hpp"
#include "type_traits.hpp"
#include "architecture.hpp"
#include <complex>
#include <cmath>
#include <limits>
#include <span>
#include <tuple>
#include <concepts>

#if HI_COMPILER == HI_CC_MSVC
#include <intrin.h>
#endif
#if HI_PROCESSOR == HI_CPU_X64
#include <immintrin.h>
#endif

hi_warning_push();
// C4702 unreachable code: Suppressed due intrinsics and std::is_constant_evaluated()
hi_warning_ignore_msvc(4702);

namespace hi {

/** Get a bit from an array of unsigned integers.
 * The integers are arranged in little-endian order.
 *
 * @param lhs The array of integers from which to take the bit.
 * @param index The index of the bit
 * @return The value of the bit, either 0 or 1, with the same type as the integers of the array.
 */
template<std::unsigned_integral T>
[[nodiscard]] hi_force_inline constexpr T get_bit(T const *lhs, std::size_t index) noexcept
{
    constexpr std::size_t bits_per_digit = sizeof(T) * CHAR_BIT;

    hilet digit_count = index / bits_per_digit;
    hilet bit_count = index % bits_per_digit;

    return (lhs[digit_count] >> bit_count) & 1;
}

/** Set a bit from an array of unsigned integers.
 * The integers are arranged in little-endian order.
 *
 * @param r The array of integers on which to set the bit.
 * @param index The index of the bit
 * @param value The value of the bit, either 0 or 1.
 */
template<std::unsigned_integral T>
constexpr void set_bit(T *r, std::size_t index, T value = T{1}) noexcept
{
    hi_axiom(value <= 1);

    constexpr std::size_t bits_per_digit = sizeof(T) * CHAR_BIT;

    hilet digit_count = index / bits_per_digit;
    hilet bit_count = index % bits_per_digit;

    value <<= bit_count;
    hilet mask = ~(T{1} << bit_count);
    r[digit_count] = (r[digit_count] & mask) | value;
}

/** Shift logical left with carry chain.
 * @param lhs The original value
 * @param rhs The count by how much to shift lhs left.
 * @param carry The carry data to or with the lower bits.
 * @return (result, carry); the carry which can be used to pass into the next iteration.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr std::pair<T, T> sll_carry(T lhs, std::size_t rhs, T carry = T{0}) noexcept
{
    constexpr auto num_bits = sizeof(T) * CHAR_BIT;
    hilet reverse_count = num_bits - rhs;

    return {(lhs << rhs) | carry, lhs >> reverse_count};
}

/** Shift logical right with carry chain.
 * @param lhs The original value
 * @param rhs The count by how much to shift lhs right.
 * @param carry The carry data to or with the lower bits.
 * @return (result, carry); the carry which can be used to pass into the next iteration.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr std::pair<T, T> srl_carry(T lhs, std::size_t rhs, T carry = T{0}) noexcept
{
    constexpr auto num_bits = sizeof(T) * CHAR_BIT;
    hilet reverse_count = num_bits - rhs;

    return {(lhs >> rhs) | carry, lhs << reverse_count};
}

/** Shift arithmetic right with carry chain.
 * @param lhs The original value
 * @param rhs The count by how much to shift lhs right.
 * @return (result, carry); the carry which can be used to pass into the next iteration.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr std::pair<T, T> sra_carry(T lhs, std::size_t rhs) noexcept
{
    using S = std::make_signed_t<T>;

    constexpr auto num_bits = sizeof(T) * CHAR_BIT;
    hilet reverse_count = num_bits - rhs;

    return {(static_cast<S>(lhs) >> rhs), lhs << reverse_count};
}

/** Add two numbers with carry chain.
 * @param lhs The left hand side
 * @param rhs The right hand side
 * @param carry From the previous add in the chain
 * @return (result, carry) pair
 */
template<std::unsigned_integral T>
hi_force_inline constexpr std::pair<T, T> add_carry(T lhs, T rhs, T carry = T{0}) noexcept
{
    hi_axiom(carry <= 1);

    constexpr std::size_t num_bits = sizeof(T) * CHAR_BIT;

    if constexpr (has_uintxx_v<num_bits * 2>) {
        // We can use a native type that has double the size.
        using U = make_uintxx_t<num_bits * 2>;

        hilet r = static_cast<U>(lhs) + static_cast<U>(rhs) + static_cast<U>(carry);
        return {static_cast<T>(r), static_cast<T>(r >> num_bits)};

    } else if (not std::is_constant_evaluated()) {
#if HI_COMPILER == HI_CC_MSVC
        uint64_t r;
        hilet c = _addcarry_u64(static_cast<unsigned char>(carry), lhs, rhs, &r);
        return {r, static_cast<T>(c)};
#endif
    }

    // Carry can directly be added the sum without a double overflow.
    hilet r = static_cast<T>(lhs + rhs + carry);
    hilet c = static_cast<T>(r < lhs or r < rhs);
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
hi_force_inline constexpr std::pair<T, T> mul_carry(T lhs, T rhs, T carry = T{0}, T accumulator = T{0}) noexcept
{
    constexpr std::size_t num_bits = sizeof(T) * CHAR_BIT;

    if constexpr (has_uintxx_v<num_bits * 2>) {
        // We can use a native type that has double the size.
        using U = make_uintxx_t<num_bits * 2>;

        hilet r = static_cast<U>(lhs) * static_cast<U>(rhs) + static_cast<U>(carry) + static_cast<U>(accumulator);
        return {static_cast<T>(r), static_cast<T>(r >> num_bits)};

    } else if (not std::is_constant_evaluated()) {
#if HI_COMPILER == HI_CC_MSVC
        if constexpr (sizeof(T) == 8) {
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

    constexpr std::size_t num_half_bits = num_bits / 2;
    constexpr T half_mask = (T{1} << num_half_bits) - T{1};

    hilet A = lhs >> num_half_bits;
    hilet B = lhs & half_mask;
    hilet C = rhs >> num_half_bits;
    hilet D = rhs & half_mask;
    hilet AC = A * C;
    hilet AD = A * D;
    hilet BC = B * C;
    hilet BD = B * D;

    // Provisional result.
    auto hi = AC;
    auto lo = BD;
    auto c = T{0};

    // AD and BC are shifted half way across the lo and hi of the result.
    hilet AD_lo = AD << num_half_bits;
    hilet AD_hi = AD >> num_half_bits;
    hilet BC_lo = BC << num_half_bits;
    hilet BC_hi = BC >> num_half_bits;

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
 * @return The result.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr T wide_div(T lhs_lo, T lhs_hi, T rhs) noexcept
{
    if constexpr (sizeof(T) == 1) {
        hilet lhs = static_cast<uint16_t>(lhs_hi) << 8 | static_cast<uint16_t>(lhs_lo);
        return narrow_cast<uint8_t>(lhs / rhs);

    } else if constexpr (sizeof(T) == 2) {
        hilet lhs = static_cast<uint32_t>(lhs_hi) << 16 | static_cast<uint32_t>(lhs_lo);
        return narrow_cast<uint16_t>(lhs / rhs);

    } else if constexpr (sizeof(T) == 4) {
        hilet lhs = static_cast<uint64_t>(lhs_hi) << 32 | static_cast<uint64_t>(lhs_lo);
        return narrow_cast<uint32_t>(lhs / rhs);

    } else if constexpr (sizeof(T) == 8) {
#if HI_COMPILER == HI_CC_MSVC
        uint64_t remainder;
        return _udiv128(lhs_hi, lhs_lo, rhs, &remainder);

#elif HI_COMPILER == HI_CC_CLANG || HI_COMPILER == HI_CC_GCC
        hilet lhs = static_cast<__uint128_t>(lhs_hi) << 64 | static_cast<__uint128_t>(lhs_lo);
        return narrow_cast<uint64_t>(lhs / rhs);
#else
#error "Not implemented"
#endif
    }
}

/** Bit scan reverse.
 *
 * @param lhs The array of unsigned integers to find the highest set bit off.
 * @param n The number of unsigned integers in the array.
 * @return index of leading one, or -1 when rhs is zero.
 */
template<std::unsigned_integral T>
[[nodiscard]] hi_force_inline constexpr ssize_t bsr_carry_chain(T const *lhs, std::size_t n) noexcept
{
    constexpr std::size_t bits_per_digit = sizeof(T) * CHAR_BIT;

    for (ssize_t i = static_cast<ssize_t>(n) - 1; i >= 0; i--) {
        auto tmp = std::countl_zero(lhs[i]);
        if (tmp < bits_per_digit) {
            return i * bits_per_digit + bits_per_digit - tmp - 1;
        }
    }
    return -1;
}

/** Invert unsigned integers using a carry-chain
 * Technically this is not an carry chain.
 *
 * @param r The result of the inversion.
 * @param rhs The right hand side operand.
 * @param n The number of digits of @a r and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void invert_carry_chain(T *r, T const *rhs, std::size_t n) noexcept
{
    for (std::size_t i = 0; i != n; ++i) {
        r[i] = ~rhs[i];
    }
}

/** shift logical right using a carry-chain
 *
 * @param r The result of the logical-shift-right.
 * @param lhs The left hand side operand of the lsr.
 * @param rhs The right hand side operand of the lsr, the number of bits to shift.
 * @param n The number of digits of @a r, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void sll_carry_chain(T *r, T const *lhs, std::size_t rhs, std::size_t n) noexcept
{
    constexpr std::size_t bits_per_digit = sizeof(T) * CHAR_BIT;

    hilet digit_count = static_cast<ssize_t>(rhs / bits_per_digit);
    hilet bit_count = rhs % bits_per_digit;

    if (r != lhs or digit_count > 0) {
        ssize_t i;
        for (i = static_cast<ssize_t>(n) - 1; i >= digit_count; --i) {
            r[i] = lhs[i - digit_count];
        }
        for (; i >= 0; --i) {
            r[i] = T{0};
        }
    }

    if (bit_count > 0) {
        auto carry = T{0};
        for (std::size_t i = 0; i != n; ++i) {
            std::tie(r[i], carry) = sll_carry(r[i], bit_count, carry);
        }
    }
}

/** shift logical right using a carry-chain
 *
 * @param r The result of the logical-shift-right.
 * @param lhs The left hand side operand of the lsr.
 * @param rhs The right hand side operand of the lsr, the number of bits to shift.
 * @param n The number of digits of @a r, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void srl_carry_chain(T *r, T const *lhs, std::size_t rhs, std::size_t n) noexcept
{
    constexpr std::size_t bits_per_digit = sizeof(T) * CHAR_BIT;

    hilet digit_count = rhs / bits_per_digit;
    hilet bit_count = rhs % bits_per_digit;

    if (r != lhs or digit_count > 0) {
        std::size_t i = 0;
        for (; i != (n - digit_count); ++i) {
            r[i] = lhs[i + digit_count];
        }
        for (; i != n; ++i) {
            r[i] = T{0};
        }
    }

    if (bit_count > 0) {
        auto carry = T{0};

        for (ssize_t i = static_cast<ssize_t>(n) - digit_count - 1; i >= 0; --i) {
            std::tie(r[i], carry) = srl_carry(r[i], bit_count, carry);
        }
    }
}

/** shift arithmetic right using a carry-chain
 * This sign-extends the left most bit.
 *
 * @param r The result of the logical-shift-right.
 * @param lhs The left hand side operand of the lsr.
 * @param rhs The right hand side operand of the lsr, the number of bits to shift.
 * @param n The number of digits of @a r, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void sra_carry_chain(T *r, T const *lhs, std::size_t rhs, std::size_t n) noexcept
{
    using S = std::make_signed_t<T>;
    constexpr std::size_t bits_per_digit = sizeof(T) * CHAR_BIT;

    hilet digit_count = rhs / bits_per_digit;
    hilet bit_count = rhs % bits_per_digit;

    if (r != lhs or digit_count > 0) {
        hi_axiom(digit_count < n);

        std::size_t i = 0;
        for (; i != (n - digit_count); ++i) {
            r[i] = lhs[i + digit_count];
        }

        // Sign extent the digits that are unused after a large shift.
        hilet sign = lhs[n - 1] < 0 ? S{-1} : S{0};
        for (; i != n; ++i) {
            r[i] = sign;
        }
    }

    if (bit_count > 0) {
        hi_axiom(n > 0);
        auto carry = T{};

        // The most significant digit is sign extended.
        ssize_t i = static_cast<ssize_t>(n) - digit_count - 1;
        std::tie(r[i], carry) = sra_carry(r[i], bit_count);
        --i;

        // The rest of the digits pass through the carry.
        for (; i >= 0; --i) {
            std::tie(r[i], carry) = srl_carry(r[i], bit_count, carry);
        }
    }
}

/** and-operation unsigned integers using a carry-chain
 *
 * @param r The result of the and-operation.
 * @param lhs The left hand side operand of the and-operation.
 * @param rhs The right hand side operand of the and-operation.
 * @param n The number of digits of @a r, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void and_carry_chain(T *r, T const *lhs, T const *rhs, std::size_t n) noexcept
{
    for (std::size_t i = 0; i != n; ++i) {
        r[i] = lhs[i] & rhs[i];
    }
}

/** or-operation unsigned integers using a carry-chain
 *
 * @param r The result of the and-operation.
 * @param lhs The left hand side operand of the or-operation.
 * @param rhs The right hand side operand of the or-operation.
 * @param n The number of digits of @a r, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void or_carry_chain(T *r, T const *lhs, T const *rhs, std::size_t n) noexcept
{
    for (std::size_t i = 0; i != n; ++i) {
        r[i] = lhs[i] | rhs[i];
    }
}

/** xor-operation unsigned integers using a carry-chain
 *
 * @param r The result of the and-operation.
 * @param lhs The left hand side operand of the xor-operation.
 * @param rhs The right hand side operand of the xor-operation.
 * @param n The number of digits of @a r, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void xor_carry_chain(T *r, T const *lhs, T const *rhs, std::size_t n) noexcept
{
    for (std::size_t i = 0; i != n; ++i) {
        r[i] = lhs[i] ^ rhs[i];
    }
}

template<std::unsigned_integral T>
[[nodiscard]] hi_force_inline constexpr bool eq_carry_chain(T const *lhs, T const *rhs, std::size_t n) noexcept
{
    for (std::size_t i = 0; i != n; ++i) {
        if (lhs[i] != rhs[i]) {
            return false;
        }
    }
    return true;
}

template<std::unsigned_integral T>
[[nodiscard]] hi_force_inline constexpr bool ne_carry_chain(T const *lhs, T const *rhs, std::size_t n) noexcept
{
    return not eq_carry_chain(lhs, rhs, n);
}

template<std::unsigned_integral T>
[[nodiscard]] hi_force_inline constexpr std::strong_ordering
cmp_unsigned_carry_chain(T const *lhs, T const *rhs, std::size_t n) noexcept
{
    for (ssize_t i = static_cast<ssize_t>(n) - 1; i >= 0; --i) {
        hilet r = lhs[i] <=> rhs[i];
        if (r != std::strong_ordering::equal) {
            return r;
        }
    }
    return std::strong_ordering::equal;
}

template<std::unsigned_integral T>
[[nodiscard]] hi_force_inline constexpr std::strong_ordering cmp_signed_carry_chain(T const *lhs, T const *rhs, std::size_t n) noexcept
{
    using S = std::make_signed_t<T>;

    // Compare the ms-digit using signed comparison, because it includes the sign-bit
    if (n > 0) {
        hilet r = static_cast<S>(lhs[n - 1]) <=> static_cast<S>(rhs[n - 1]);
        if (r != std::strong_ordering::equal) {
            return r;
        }
    }

    // At this point both values have the same sign, and since the rest of the digits
    // do not have a sign bit, use unsigned comparison.
    for (ssize_t i = static_cast<ssize_t>(n) - 2; i >= 0; --i) {
        hilet r = lhs[i] <=> rhs[i];
        if (r != std::strong_ordering::equal) {
            return r;
        }
    }
    return std::strong_ordering::equal;
}

template<std::unsigned_integral T>
[[nodiscard]] hi_force_inline constexpr bool lt_unsigned_carry_chain(T const *lhs, T const *rhs, std::size_t n) noexcept
{
    return cmp_unsigned_carry_chain(lhs, rhs, n) == std::strong_ordering::less;
}

template<std::unsigned_integral T>
[[nodiscard]] hi_force_inline constexpr bool gt_unsigned_carry_chain(T const *lhs, T const *rhs, std::size_t n) noexcept
{
    return cmp_unsigned_carry_chain(lhs, rhs, n) == std::strong_ordering::greater;
}

template<std::unsigned_integral T>
[[nodiscard]] hi_force_inline constexpr bool ge_unsigned_carry_chain(T const *lhs, T const *rhs, std::size_t n) noexcept
{
    return lt_unsigned_carry_chain(rhs, lhs, n);
}

template<std::unsigned_integral T>
[[nodiscard]] hi_force_inline constexpr bool le_unsigned_carry_chain(T const *lhs, T const *rhs, std::size_t n) noexcept
{
    return gt_unsigned_carry_chain(rhs, lhs, n);
}

/** Negate unsigned integers using a carry-chain
 * This is a two's compliment negate.
 *
 * @param r The result of the addition.
 * @param rhs The left hand side operand of the addition.
 * @param n The number of digits of @a r, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void neg_carry_chain(T *r, T const *rhs, std::size_t n) noexcept
{
    auto carry = T{1};
    for (std::size_t i = 0; i != n; ++i) {
        std::tie(r[i], carry) = add_carry(~rhs[i], T{0}, carry);
    }
}

/** Add unsigned integers using a carry-chain
 *
 * @param r The result of the addition.
 * @param lhs The left hand side operand of the addition.
 * @param rhs The right hand side operand of the addition.
 * @param n The number of digits of @a r, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void add_carry_chain(T *r, T const *lhs, T const *rhs, std::size_t n) noexcept
{
    auto carry = T{0};
    for (std::size_t i = 0; i != n; ++i) {
        std::tie(r[i], carry) = add_carry(lhs[i], rhs[i], carry);
    }
}

/** Subtract unsigned integers using a carry-chain
 *
 * @param r The result of the addition.
 * @param lhs The left hand side operand of the addition.
 * @param rhs The right hand side operand of the addition.
 * @param n The number of digits of @a r, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void sub_carry_chain(T *r, T const *lhs, T const *rhs, std::size_t n) noexcept
{
    auto carry = T{1};
    for (std::size_t i = 0; i != n; ++i) {
        std::tie(r[i], carry) = add_carry(lhs[i], ~rhs[i], carry);
    }
}

/** Multiply unsigned integers using a carry-chain
 *
 * @note @a r May not alias with @a lhs or @a rhs.
 * @param r The result of the multiplication.
 * @param lhs The left hand side operand.
 * @param rhs The right hand side operand.
 * @param n The number of digits of @a r, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
hi_force_inline constexpr void mul_carry_chain(T *hi_restrict r, T const *lhs, T const *rhs, std::size_t n) noexcept
{
    hi_axiom(r != lhs and r != rhs);

    for (auto rhs_index = 0; rhs_index < n; rhs_index++) {
        hilet rhs_digit = rhs[rhs_index];

        T carry = 0;
        for (auto lhs_index = 0; (lhs_index + rhs_index) < n; lhs_index++) {
            hilet lhs_digit = lhs[lhs_index];

            T result;
            T accumulator = r[rhs_index + lhs_index];
            std::tie(result, carry) = mul_carry(lhs_digit, rhs_digit, carry, accumulator);
            r[rhs_index + lhs_index] = result;
        }
    }
}

/** Divide unsigned integers using a carry-chain
 * This function does a bit-wise division.
 *
 * @note @a quotient and @a remainder may not alias with @a lhs or @a rhs or with each other.
 * @param quotient The result of the division.
 * @param remainder The remainder of the division.
 * @param lhs The left hand side operand.
 * @param rhs The right hand side operand.
 * @param n The number of digits of @a quotient, @a remainder, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
constexpr void div_carry_chain(T *hi_restrict quotient, T *hi_restrict remainder, T const *lhs, T const *rhs, std::size_t n) noexcept
{
    hi_axiom(quotient != lhs and quotient != rhs and quotient != remainder);
    hi_axiom(remainder != lhs and remainder != rhs);

    hilet nr_bits = static_cast<ssize_t>(n * sizeof(T) * CHAR_BIT);

    for (ssize_t i = nr_bits - 1; i >= 0; i--) {
        sll_carry_chain(remainder, remainder, 1, n);
        remainder[0] |= get_bit(lhs, i);
        if (ge_unsigned_carry_chain(remainder, rhs, n)) {
            sub_carry_chain(remainder, remainder, rhs, n);
            set_bit(quotient, i);
        }
    }
}

/** signed divide unsigned integers using a carry-chain
 * This function does a bit-wise division.
 * This function will allocate memory when one or both operands are negative.
 *
 * @note @a quotient and @a remainder may not alias with @a lhs or @a rhs or with each other.
 * @param quotient The result of the division.
 * @param remainder The remainder of the division. The remainder has same sign as @a lhs.
 * @param lhs The left hand side operand.
 * @param rhs The right hand side operand.
 * @param n The number of digits of @a quotient, @a remainder, @a lhs and @a rhs.
 */
template<std::unsigned_integral T>
constexpr void
signed_div_carry_chain(T *hi_restrict quotient, T *hi_restrict remainder, T const *lhs, T const *rhs, std::size_t n) noexcept
{
    hi_axiom(n > 0);
    hi_axiom(quotient != lhs and quotient != rhs and quotient != remainder);
    hi_axiom(remainder != lhs and remainder != rhs);

    using signed_type = std::make_signed_t<T>;

    hilet lhs_is_negative = static_cast<signed_type>(lhs[n - 1]) < 0;
    hilet rhs_is_negative = static_cast<signed_type>(rhs[n - 1]) < 0;

    auto tmp = std::vector<T>{};
    if (lhs_is_negative or rhs_is_negative) {
        // Allocate room for negative lhs and rhs together, so only one allocation is needed.
        tmp.resize(n * 2);

        if (lhs_is_negative) {
            // Negate lhs and point it to the tmp array.
            T *p = tmp.data();
            neg_carry_chain(p, lhs, n);
            lhs = p;
        }

        if (rhs_is_negative) {
            // Negate rhs and point it to the tmp array.
            T *p = tmp.data() * n;
            neg_carry_chain(p, rhs, n);
            rhs = p;
        }
    }

    div_carry_chain(quotient, remainder, lhs, rhs, n);

    if (lhs_is_negative != rhs_is_negative) {
        // Negate the result if the sign of lhs and rhs are different.
        neg_carry_chain(quotient, quotient, n);
    }
    if (lhs_is_negative) {
        // Remainder has same sign as divisor.
        neg_carry_chain(remainder, remainder, n);
    }
}

} // namespace hi

hi_warning_pop();
