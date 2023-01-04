// Copyright Take Vos 2022, 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd_utility.hpp"
#include "../assert.hpp"
#include <span>
#include <array>
#include <ostream>

namespace hi { inline namespace v1 {

#ifdef HI_HAS_AVX2

/** A double x 4 (__m256) SSE register.
 *
 *
 * When loading and storing from memory this is the order of the element in the register
 *
 * ```
 *   lo           hi lo           hi lo           hi lo           hi
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  | element 0/a/x | element 1/b/y | element 2/c/z | element 3/d/w |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   0             7 8            15 16           23 24           31   memory address.
 * ```
 *
 * In the function below a `mask` values least-significant-bit corresponds to element 0.
 *
 */
class simd_i64x4 {
public:
    using value_type = int64_t;
    constexpr static size_t size = 4;
    using array_type = std::array<value_type, size>;
    using register_type = __m256i;

    simd_i64x4(simd_i64x4 const&) noexcept = default;
    simd_i64x4(simd_i64x4&&) noexcept = default;
    simd_i64x4& operator=(simd_i64x4 const&) noexcept = default;
    simd_i64x4& operator=(simd_i64x4&&) noexcept = default;

    /** Initialize all elements to zero.
     */
    simd_i64x4() noexcept : v(_mm256_setzero_si256()) {}

    /** Initialize the element to the values in the arguments.
     *
     * @param a The value for element 0.
     * @param b The value for element 1.
     * @param c The value for element 2.
     * @param d The value for element 3.
     */
    [[nodiscard]] simd_i64x4(
        value_type a,
        value_type b = value_type{0},
        value_type c = value_type{0},
        value_type d = value_type{0}) noexcept :
        v(_mm256_set_epi64x(d, c, b, a))
    {
    }

    [[nodiscard]] explicit simd_i64x4(value_type const *other) noexcept : v(_mm256_loadu_si256(other)) {}

    void store(value_type *out) const noexcept
    {
        hi_axiom_not_null(out);
        _mm256_storeu_si256(out, v);
    }

    [[nodiscard]] explicit simd_i64x4(void const *other) noexcept : v(_mm256_loadu_si256(static_cast<value_type const *>(other))) {}

    void store(void *out) const noexcept
    {
        hi_axiom_not_null(out);
        _mm256_storeu_si256(static_cast<value_type *>(out), v);
    }

    [[nodiscard]] explicit simd_i64x4(std::span<value_type const> other) noexcept
    {
        hi_axiom(other.size() >= 4);
        v = _mm256_loadu_si256(other.data());
    }

    void store(std::span<value_type> out) const noexcept
    {
        hi_axiom(out.size() >= 4);
        _mm256_storeu_si256(out.data(), v);
    }

    [[nodiscard]] explicit simd_i64x4(array_type other) noexcept : v(_mm256_loadu_si256(other.data())) {}

    [[nodiscard]] explicit operator array_type() const noexcept
    {
        auto r = array_type{};
        _mm256_storeu_si256(r.data(), v);
        return r;
    }

    [[nodiscard]] explicit simd_i64x4(register_type other) noexcept : v(other) {}

    [[nodiscard]] explicit operator register_type() const noexcept
    {
        return v;
    }

    /** Check if all elements are zero.
     */
    [[nodiscard]] bool empty() const noexcept
    {
        return eq(*this, simd_i64x4{}).mask() == 0b1111;
    }

    /** Check if any element is non-zero.
     */
    [[nodiscard]] explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Broadcast a single value to all the elements.
     *
     * ```
     * r[0] = a
     * r[1] = a
     * r[2] = a
     * r[3] = a
     * ```
     */
    [[nodiscard]] static simd_i64x4 broadcast(value_type a) noexcept
    {
        return simd_i64x4{_mm256_set1_epi64x(a)};
    }

    /** Broadcast the first element to all the elements.
     *
     * ```
     * r[0] = a[0]
     * r[1] = a[0]
     * r[2] = a[0]
     * r[3] = a[0]
     * ```
     */
    [[nodiscard]] static simd_i64x4 broadcast(simd_i64x4 a) noexcept
    {
        return simd_i64x4{_mm256_permute4x64_epi64(a.v, 0b00'00'00'00)}; 
    }

    /** Create a vector with all the bits set.
     */
    [[nodiscard]] static simd_i64x4 ones() noexcept
    {
        return eq(simd_i64x4{}, simd_i64x4{});
    }

    /** Concatenate the top bit of each element.
     */
    [[nodiscard]] size_t mask() const noexcept
    {
        return narrow_cast<size_t>(_mm256_movemask_epi64(_mm256_castpd_si256(v)));
    }

    /** Compare if all elements in both vectors are equal.
     *
     * This operator does a bit-wise compare. It does not handle NaN in the same
     * way as IEEE-754. This is because when you comparing two vectors
     * having a NaN in one of the elements does not invalidate the complete vector.
     */
    [[nodiscard]] friend bool operator==(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return _mm256_movemask_epi64(_mm256_cmpeq_epi64(a.v, b.v)) == 0b1111;
    }

    [[nodiscard]] friend simd_i64x4 eq(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return simd_i64x4{_mm256_cmpeq_epi64(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i64x4 ne(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return ~eq(a, b);
    }

    [[nodiscard]] friend simd_i64x4 lt(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return ~ge(a, b);
    }

    [[nodiscard]] friend simd_i64x4 gt(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return simd_i64x4{_mm256_cmpgt_epi64(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i64x4 le(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return ~gt(a, b);
    }

    [[nodiscard]] friend simd_i64x4 ge(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return gt(a, b) | eq(a, b);
    }

    [[nodiscard]] friend simd_i64x4 operator+(simd_i64x4 a) noexcept
    {
        return a;
    }

    [[nodiscard]] friend simd_i64x4 operator-(simd_i64x4 a) noexcept
    {
        return simd_i64x4{} - a;
    }

    [[nodiscard]] friend simd_i64x4 operator+(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return simd_i64x4{_mm256_add_epi64(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i64x4 operator-(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return simd_i64x4{_mm256_sub_epi64(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i64x4 operator&(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return simd_i64x4{_mm256_and_si256(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i64x4 operator|(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return simd_i64x4{_mm256_or_si256(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i64x4 operator^(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return simd_i64x4{_mm256_xor_si256(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i64x4 operator~(simd_i64x4 a) noexcept
    {
        return not_and(a, ones());
    }

    [[nodiscard]] friend simd_i64x4 min(simd_i64x4 a, simd_i64x4 b) noexcept
    {
    }

    [[nodiscard]] friend simd_i64x4 max(simd_i64x4 a, simd_i64x4 b) noexcept
    {
    }

    [[nodiscard]] friend simd_i64x4 abs(simd_i64x4 a) noexcept
    {
    }

    /** Set elements to zero.
     *
     * @tparam Mask A bit mask corresponding to each element.
     * @param a The value to modify.
     * @return argument @a with elements set to zero where the corresponding @a Mask bit was '1'.
     */
    template<size_t Mask>
    [[nodiscard]] friend simd_i64x4 set_zero(simd_i64x4 a) noexcept
    {
        static_assert(Mask <= 0b1111);

        if constexpr (Mask == 0b0000) {
            return a;
        } else if constexpr (Mask == 0b1111) {
            return {};
        } else {
            return blend(a, simd_i64x4{}, Mask)};
        }
    }

    /** Insert a value into an element of a vector.
     *
     * @tparam Index the index of the element where insert the value.
     * @param a The vector to insert the value into.
     * @param b The value to insert.
     * @return The vector with the inserted value.
     */
    template<size_t Index>
    [[nodiscard]] friend simd_i64x4 insert(simd_i64x4 a, value_type b) noexcept
    {
        static_assert(Index < 4);

        constexpr auto mask = 1 << Index;
        return blend(a, broadcast(b), mask)};
    }

    /** Extract an element from a vector.
     *
     * @tparam Index the index of the element.
     * @param a The vector to select the element from.
     * @return The value of the selected element.
     */
    template<size_t Index>
    [[nodiscard]] friend value_type get(simd_i64x4 a) noexcept
    {
        static_assert(Index < size);

        return _mm256_extract_epi64(a, Index);
    }

    /** Select elements from two vectors.
     *
     * @tparam Mask A mask to select bits from @a a when '0'; or @a b when '1'. The
     *         lsb corresponds with element zero.
     * @param a A vector for which element are selected when the bit in @a Mask is '0'.
     * @param b A vector for which element are selected when the bit in @a Mask is '1'.
     * @return A vector with element selected from @a a and @a b
     */
    template<size_t Mask>
    [[nodiscard]] friend simd_i64x4 blend(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        static_assert(Mask <= 0b1111);

        if constexpr (Mask == 0b0000) {
            return a;
        } else if constexpr (Mask == 0b1111) {
            return b;
        } else {
            // clang-format off
            constexpr auto dmask =
                (Mask & 0b0001) | ((Mask & 0b0001) << 1) |
                ((Mask & 0b0010) << 1) | ((Mask & 0b0010) << 2) |
                ((Mask & 0b0100) << 2) | ((Mask & 0b0100) << 3) |
                ((Mask & 0b1000) << 3) | ((Mask & 0b1000) << 4) |
            // clang-format on
            return simd_i64x4{_mm256_blend_epi32(a.v, b.v, dmask)};
        }
    }

    /** Permute elements, ignoring numeric elements.
     *
     * The characters in @a SourceElements mean the following:
     * - 'a' - 'p': The indices to elements 0 and 15 of @a a.
     * - 'x', 'y', 'z', 'w'': The indices to elements 0, 1, 2, 3 of @a a.
     * - Any other character is treated as if the original element was selected.
     *
     * @tparam SourceElements A string representing the order of elements. First character
     *         matches the first element.
     * @param a The vector to swizzle the elements
     * @returns A vector with the elements swizzled.
     */
    template<fixed_string SourceElements>
    [[nodiscard]] friend simd_i64x4 permute(simd_i64x4 a) noexcept
    {
        static_assert(SourceElements.size() == size);
        constexpr auto order = detail::simd_swizzle_to_packed_indices<SourceElements, size>();

        if constexpr (order == 0b11'10'01'00) {
            return a;
        } else {
            return simd_i64x4{_mm256_permute4x64_epi64(a.v, order)};
        }
    }

    /** Swizzle elements.
     *
     * The elements are swizzled in the order specified in @a SourceElements.
     * Each character in @a SourceElements is a index to an element in @a a or
     * a numeric value.
     *
     * The characters in @a SourceElements mean the following:
     * - 'a' - 'p': The indices to elements 0 and 15 of @a a.
     * - 'x', 'y', 'z', 'w'': The indices to elements 0, 1, 2, 3 of @a a.
     * - '0', '1': The values 0 and 1.
     *
     * @tparam SourceElements A string representing the order of elements. First character
     *         matches the first element.
     * @param a The vector to swizzle the elements
     * @returns A vector with the elements swizzled.
     */
    template<fixed_string SourceElements>
    [[nodiscard]] friend simd_i64x4 swizzle(simd_i64x4 a) noexcept
    {
        static_assert(SourceElements.size() == size);
        constexpr auto one_mask = detail::simd_swizzle_to_mask<SourceElements, size, '1'>();
        constexpr auto zero_mask = detail::simd_swizzle_to_mask<SourceElements, size, '0'>();
        constexpr auto number_mask = one_mask | zero_mask;

        if constexpr (number_mask == 0b1111) {
            // Swizzle was /[01][01][01][01]/.
            return swizzle_numbers<SourceElements>();

        } else if constexpr (number_mask == 0b0000) {
            // Swizzle was /[^01][^01][^01][^01]/.
            return permute<SourceElements>(a);

#ifdef HI_HAS_SSE4_1
        } else if constexpr (number_mask == zero_mask) {
            // Swizzle was /[^1][^1][^1][^1]/.
            hilet ordered = permute<SourceElements>(a);
            return set_zero<zero_mask>(ordered);
#endif

        } else {
            hilet ordered = permute<SourceElements>(a);
            hilet numbers = swizzle_numbers<SourceElements>();
            return blend<number_mask>(ordered, numbers);
        }
    }

    /** not followed by and.
     *
     * r = ~a & b
     *
     */
    [[nodiscard]] friend simd_i64x4 not_and(simd_i64x4 a, simd_i64x4 b) noexcept
    {
        return simd_i64x4{_mm256_andnot_si256(a.v, b.v)};
    }

    friend std::ostream& operator<<(std::ostream& a, simd_i64x4 b) noexcept
    {
        return a << "(" << get<0>(b) << ", " << get<1>(b) << ", " << get<2>(b) << ", " << get<3>(b) << ")";
    }

private:
    register_type v;

    template<fixed_string SourceElements>
    [[nodiscard]] static simd_i64x4 swizzle_numbers() noexcept
    {
        constexpr auto one_mask = detail::simd_swizzle_to_mask<SourceElements, size, '1'>();
        constexpr auto zero_mask = detail::simd_swizzle_to_mask<SourceElements, size, '0'>();
        constexpr auto number_mask = one_mask | zero_mask;
        constexpr auto alpha_mask = ~number_mask & 0b1111;

        if constexpr ((zero_mask | alpha_mask) == 0b1111) {
            return {};

        } else if constexpr ((one_mask | alpha_mask) == 0b1111) {
            return broadcast(1);

        } else {
            return simd_i64x4{
                to_bool(one_mask & 0b0001) ? 1 : 0,
                to_bool(one_mask & 0b0010) ? 1 : 0,
                to_bool(one_mask & 0b0100) ? 1 : 0,
                to_bool(one_mask & 0b1000) ? 1 : 0};
        }
    }
};

template<>
struct low_level_simd<int64_t, 4> : std::true_type {
    using type = simd_i64x4;
};

#endif

}} // namespace hi::v1
