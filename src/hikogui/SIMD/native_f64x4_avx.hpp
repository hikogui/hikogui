// Copyright Take Vos 2022, 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "native_simd_utility.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <span>
#include <array>
#include <ostream>

#ifdef HI_HAS_SSE
#include <xmmintrin.h>
#endif
#ifdef HI_HAS_SSE2
#include <emmintrin.h>
#endif
#ifdef HI_HAS_SSE3
#include <pmmintrin.h>
#endif
#ifdef HI_HAS_SSSE3
#include <tmmintrin.h>
#endif
#ifdef HI_HAS_SSE4_1
#include <smmintrin.h>
#endif
#ifdef HI_HAS_SSE4_2
#include <nmmintrin.h>
#endif
#ifdef HI_HAS_AVX
#include <immintrin.h>
#endif

hi_export_module(hikogui.SIMD : native_f64x4_avx);


hi_export namespace hi { inline namespace v1 {

#ifdef HI_HAS_AVX

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
template<>
struct native_simd<double,4> {
    using value_type = double;
    constexpr static size_t size = 4;
    using array_type = std::array<value_type, size>;
    using register_type = __m256d;

    register_type v;

    native_simd(native_simd const&) noexcept = default;
    native_simd(native_simd&&) noexcept = default;
    native_simd& operator=(native_simd const&) noexcept = default;
    native_simd& operator=(native_simd&&) noexcept = default;

    /** Initialize all elements to zero.
     */
    native_simd() noexcept : v(_mm256_setzero_pd()) {}

    [[nodiscard]] explicit native_simd(register_type other) noexcept : v(other) {}

    [[nodiscard]] explicit operator register_type() const noexcept
    {
        return v;
    }

    /** Initialize the element to the values in the arguments.
     *
     * @param a The value for element 0.
     * @param b The value for element 1.
     * @param c The value for element 2.
     * @param d The value for element 3.
     */
    [[nodiscard]] native_simd(
        value_type a,
        value_type b = value_type{0},
        value_type c = value_type{0},
        value_type d = value_type{0}) noexcept :
        v(_mm256_set_pd(d, c, b, a))
    {
    }

    [[nodiscard]] explicit native_simd(value_type const *other) noexcept : v(_mm256_loadu_pd(other)) {}

    void store(value_type *out) const noexcept
    {
        hi_axiom_not_null(out);
        _mm256_storeu_pd(out, v);
    }

    [[nodiscard]] explicit native_simd(void const *other) noexcept : v(_mm256_loadu_pd(static_cast<value_type const *>(other))) {}

    void store(void *out) const noexcept
    {
        hi_axiom_not_null(out);
        _mm256_storeu_pd(static_cast<value_type *>(out), v);
    }

    [[nodiscard]] explicit native_simd(std::span<value_type const> other) noexcept
    {
        hi_axiom(other.size() >= size);
        v = _mm256_loadu_pd(other.data());
    }

    void store(std::span<value_type> out) const noexcept
    {
        hi_axiom(out.size() >= size);
        _mm256_storeu_pd(out.data(), v);
    }

    [[nodiscard]] explicit native_simd(array_type other) noexcept : v(_mm256_loadu_pd(other.data())) {}

    [[nodiscard]] explicit operator array_type() const noexcept
    {
        auto r = array_type{};
        _mm256_storeu_pd(r.data(), v);
        return r;
    }

    [[nodiscard]] explicit native_simd(native_simd<float, 4> const& a) noexcept;
    [[nodiscard]] explicit native_simd(native_simd<int32_t, 4> const& a) noexcept;
    //[[nodiscard]] explicit native_simd(native_f64x2 const &a, native_f64x2 const &b) noexcept; 

    /** Broadcast a single value to all the elements.
     *
     * ```
     * r[0] = a
     * r[1] = a
     * r[2] = a
     * r[3] = a
     * ```
     */
    [[nodiscard]] static native_simd broadcast(value_type a) noexcept
    {
        return native_simd{_mm256_set1_pd(a)};
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
    [[nodiscard]] static native_simd broadcast(native_simd a) noexcept
    {
#ifdef HI_HAS_AVX2
        return native_simd{_mm256_permute4x64_pd(a.v, 0b00'00'00'00)}; 
#else
        hilet tmp = _mm256_permute_pd(a.v, 0b0000);
        return native_simd{_mm256_permute2f128_pd(tmp, tmp, 0b0000'0000)};
#endif
    }

    /** Create a vector with all the bits set.
     */
    [[nodiscard]] static native_simd ones() noexcept
    {
#ifdef HI_HAS_AVX2
        auto ones = _mm256_undefined_si256();
        ones = _mm256_cmpeq_epi32(ones, ones);
        return native_simd{_mm256_castsi256_pd(ones)};
#else
        auto ones = _mm256_setzero_pd();
        ones = _mm256_cmpeq_pd(ones, ones);
        return native_simd{ones};
#endif
    }

    [[nodiscard]] static native_simd from_mask(size_t a) noexcept
    {
        hi_axiom(a <= 0b1111);

        uint64_t a_ = a;

        a_ <<= 31;
        auto tmp = _mm_cvtsi32_si128(truncate<uint32_t>(a_));
        a_ >>= 1;
        tmp = _mm_insert_epi32(tmp, truncate<uint32_t>(a_), 1);
        a_ >>= 1;
        tmp = _mm_insert_epi32(tmp, truncate<uint32_t>(a_), 2);
        a_ >>= 1;
        tmp = _mm_insert_epi32(tmp, truncate<uint32_t>(a_), 3);

        tmp = _mm_srai_epi32(tmp, 31);
        return native_simd{_mm256_castsi256_pd(_mm256_cvtepi32_epi64(tmp))};
    }

    /** Concatenate the top bit of each element.
     */
    [[nodiscard]] size_t mask() const noexcept
    {
        return narrow_cast<size_t>(_mm256_movemask_pd(v));
    }

    /** Compare if all elements in both vectors are equal.
     *
     * This operator does a bit-wise compare. It does not handle NaN in the same
     * way as IEEE-754. This is because when you comparing two vectors
     * having a NaN in one of the elements does not invalidate the complete vector.
     */
    [[nodiscard]] friend bool equal(native_simd a, native_simd b) noexcept
    {
        return _mm256_movemask_pd(_mm256_cmp_pd(a.v, b.v, _CMP_EQ_UQ)) == 0b1111;
    }

    [[nodiscard]] friend native_simd
    almost_eq(native_simd a, native_simd b, value_type epsilon = std::numeric_limits<value_type>::epsilon()) noexcept
    {
        auto abs_diff = abs(a - b);
        return abs_diff < broadcast(epsilon);
    }

    [[nodiscard]] friend bool
    almost_equal(native_simd a, native_simd b, value_type epsilon = std::numeric_limits<value_type>::epsilon())
    {
        return almost_eq(a, b, epsilon).mask() == 0b1111;
    }

    [[nodiscard]] friend native_simd operator==(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_cmp_pd(a.v, b.v, _CMP_EQ_OQ)};
    }

    [[nodiscard]] friend native_simd operator!=(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_cmp_pd(a.v, b.v, _CMP_NEQ_UQ)};
    }

    [[nodiscard]] friend native_simd operator<(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_cmp_pd(a.v, b.v, _CMP_LT_OQ)};
    }

    [[nodiscard]] friend native_simd operator>(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_cmp_pd(a.v, b.v, _CMP_GT_OQ)};
    }

    [[nodiscard]] friend native_simd operator<=(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_cmp_pd(a.v, b.v, _CMP_LE_OQ)};
    }

    [[nodiscard]] friend native_simd operator>=(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_cmp_pd(a.v, b.v, _CMP_GE_OQ)};
    }

    [[nodiscard]] friend native_simd operator+(native_simd a) noexcept
    {
        return a;
    }

    [[nodiscard]] friend native_simd operator+(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_add_pd(a.v, b.v)};
    }

    [[nodiscard]] friend native_simd operator-(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_sub_pd(a.v, b.v)};
    }

    [[nodiscard]] friend native_simd operator-(native_simd a) noexcept
    {
        return native_simd{} - a;
    }

    [[nodiscard]] friend native_simd operator*(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_mul_pd(a.v, b.v)};
    }

    [[nodiscard]] friend native_simd operator/(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_div_pd(a.v, b.v)};
    }

    [[nodiscard]] friend native_simd operator&(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_and_pd(a.v, b.v)};
    }

    [[nodiscard]] friend native_simd operator|(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_or_pd(a.v, b.v)};
    }

    [[nodiscard]] friend native_simd operator^(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_xor_pd(a.v, b.v)};
    }

    [[nodiscard]] friend native_simd operator~(native_simd a) noexcept
    {
        return not_and(a, ones());
    }

    [[nodiscard]] friend native_simd min(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_min_pd(a.v, b.v)};
    }

    [[nodiscard]] friend native_simd max(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_max_pd(a.v, b.v)};
    }

    [[nodiscard]] friend native_simd abs(native_simd a) noexcept
    {
        return not_and(broadcast(-0.0f), a);
    }

    [[nodiscard]] friend native_simd floor(native_simd a) noexcept
    {
        return native_simd{_mm256_floor_pd(a.v)};
    }

    [[nodiscard]] friend native_simd ceil(native_simd a) noexcept
    {
        return native_simd{_mm256_ceil_pd(a.v)};
    }

    template<native_rounding_mode Rounding = native_rounding_mode::current>
    [[nodiscard]] friend native_simd round(native_simd a) noexcept
    {
        return native_simd{_mm256_round_pd(a.v, std::to_underlying(Rounding))};
    }

    /** Reciprocal.
     */
    [[nodiscard]] friend native_simd rcp(native_simd a) noexcept
    {
        return native_simd{_mm256_div_pd(_mm256_set_pd(1.0, 1.0, 1.0, 1.0), a.v)};
    }

    /** Square root.
     */
    [[nodiscard]] friend native_simd sqrt(native_simd a) noexcept
    {
        return native_simd{_mm256_sqrt_pd(a.v)};
    }

    /** Reciprocal of the square root.
     *
     * This is often implemented in hardware using a much faster algorithm than
     * either the reciprocal and square root separately. But has slightly less
     * accuracy, see https://en.wikipedia.org/wiki/Fast_inverse_square_root
     */
    [[nodiscard]] friend native_simd rsqrt(native_simd a) noexcept
    {
        return rcp(sqrt(a));
    }

    /** Set elements to zero.
     *
     * @tparam Mask A bit mask corresponding to each element.
     * @param a The value to modify.
     * @return argument @a with elements set to zero where the corresponding @a Mask bit was '1'.
     */
    template<size_t Mask>
    [[nodiscard]] friend native_simd set_zero(native_simd a) noexcept
    {
        static_assert(Mask <= 0b1111);
        return blend<Mask>(a, native_simd{});
    }

    /** Insert a value into an element of a vector.
     *
     * @tparam Index the index of the element where insert the value.
     * @param a The vector to insert the value into.
     * @param b The value to insert.
     * @return The vector with the inserted value.
     */
    template<size_t Index>
    [[nodiscard]] friend native_simd insert(native_simd a, value_type b) noexcept
    {
        static_assert(Index < 4);
        return blend<1_uz << Index>(a, broadcast(b));
    }

    /** Extract an element from a vector.
     *
     * @tparam Index the index of the element.
     * @param a The vector to select the element from.
     * @return The value of the selected element.
     */
    template<size_t Index>
    [[nodiscard]] friend value_type get(native_simd a) noexcept
    {
        static_assert(Index < size);

#ifdef HI_HAS_AVX2
        return _mm256_cvtsd_f64(_mm256_permute4x64_pd(a.v, Index));

#else
        constexpr auto hi_index = Index / (size / 2);
        constexpr auto lo_index = Index % (size / 2);

        hilet hi = _mm256_extractf128_pd(a.v, hi_index);
        hilet lo = _mm_permute_pd(hi, lo_index);
        return _mm_cvtsd_f64(lo);
#endif
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
    [[nodiscard]] friend native_simd blend(native_simd a, native_simd b) noexcept
    {
        static_assert(Mask <= 0b1111);

        if constexpr (Mask == 0b0000) {
            return a;
        } else if constexpr (Mask == 0b1111) {
            return b;
        } else {
            return native_simd{_mm256_blend_pd(a.v, b.v, Mask)};
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
    [[nodiscard]] friend native_simd permute(native_simd a) noexcept
    {
        static_assert(SourceElements.size() == size);
        constexpr auto order = detail::native_swizzle_to_packed_indices<SourceElements, size>();

#if HI_HAS_AVX2
        if constexpr (order == 0b11'10'01'00) {
            return a;
        } else {
            return native_simd{_mm256_permute4x64_pd(a.v, order)};
        }

#else
        // clang-format off
        constexpr auto hi_order =
            ((order & 0b00'00'00'10) >> 1) |
            ((order & 0b00'00'10'00) >> 2) |
            ((order & 0b00'10'00'00) >> 3) |
            ((order & 0b10'00'00'00) >> 4);
        constexpr auto lo_order =
             (order & 0b00'00'00'01) |
            ((order & 0b00'00'01'00) >> 1) |
            ((order & 0b00'01'00'00) >> 2) |
            ((order & 0b01'00'00'00) >> 3);
        // clang-format on

        if constexpr (order == 0b11'10'01'00) {
            return a;
        } else if constexpr (order == 0b00'00'00'00) {
            return broadcast(a);
        } else if constexpr (hi_order == 0b1100) {
            return native_simd{_mm256_permute_pd(a.v, lo_order)};
        } else if constexpr (hi_order == 0b0011) {
            hilet tmp = _mm256_permute2f128_pd(a.v, a.v, 0b0000'0001);
            return native_simd{_mm256_permute_pd(tmp, lo_order)};
        } else if constexpr (hi_order == 0b1111) {
            hilet tmp = _mm256_permute2f128_pd(a.v, a.v, 0b0001'0001);
            return native_simd{_mm256_permute_pd(tmp, lo_order)};
        } else if constexpr (hi_order == 0b0000) {
            hilet tmp = _mm256_permute2f128_pd(a.v, a.v, 0b0000'0000);
            return native_simd{_mm256_permute_pd(tmp, lo_order)};
        } else {
            hilet hi_0 = _mm256_permute2f128_pd(a.v, a.v, 0b0000'0000);
            hilet hi_1 = _mm256_permute2f128_pd(a.v, a.v, 0b0001'0001);
            hilet lo_0 = _mm256_permute_pd(hi_0, lo_order);
            hilet lo_1 = _mm256_permute_pd(hi_1, lo_order);
            return native_simd{_mm256_blend_pd(lo_0, lo_1, hi_order)};
        }
#endif
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
    [[nodiscard]] friend native_simd swizzle(native_simd a) noexcept
    {
        static_assert(SourceElements.size() == size);
        constexpr auto one_mask = detail::native_swizzle_to_mask<SourceElements, size, '1'>();
        constexpr auto zero_mask = detail::native_swizzle_to_mask<SourceElements, size, '0'>();
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

    /** Horizontal add.
     *
     * Add elements pair-wise in both vectors, then merge the results:
     * ```
     * r[0] = a[0] + a[1]
     * r[1] = a[2] + a[3]
     * r[2] = b[0] + b[1]
     * r[3] = b[2] + b[3]
     * ```
     */
    [[nodiscard]] friend native_simd horizontal_add(native_simd a, native_simd b) noexcept
    {
        return permute<"acbd">(native_simd{_mm256_hadd_pd(a.v, b.v)});
    }

    /** Horizontal subtract.
     *
     * Subtract elements pair-wise in both vectors, then merge the results:
     * ```
     * r[0] = a[0] - a[1]
     * r[1] = a[2] - a[3]
     * r[2] = b[0] - b[1]
     * r[3] = b[2] - b[3]
     * ```
     */
    [[nodiscard]] friend native_simd horizontal_sub(native_simd a, native_simd b) noexcept
    {
        return permute<"acbd">(native_simd{_mm256_hsub_pd(a.v, b.v)});
    }

    /** Sum all elements of a vector.
     *
     * ```
     * r = broadcast(a[0] + a[1] + a[2] + a[3])
     * ```
     */
    [[nodiscard]] friend native_simd horizontal_sum(native_simd a) noexcept
    {
        hilet tmp = horizontal_add(a, a);
        return native_simd{_mm256_hadd_pd(tmp.v, tmp.v)};
    }

    /** Interleaved subtract and add elements.
     *
     * The following operations are done:
     * ```
     * r[0] = a[0] - b[0];
     * r[1] = a[1] + b[1];
     * r[2] = a[2] - b[2];
     * r[3] = a[3] + b[3];
     * ```
     *
     */
    [[nodiscard]] friend native_simd interleaved_sub_add(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_addsub_pd(a.v, b.v)};
    }

    /** not followed by and.
     *
     * r = ~a & b
     *
     */
    [[nodiscard]] friend native_simd not_and(native_simd a, native_simd b) noexcept
    {
        return native_simd{_mm256_andnot_pd(a.v, b.v)};
    }

    friend std::ostream& operator<<(std::ostream& a, native_simd b) noexcept
    {
        return a << "(" << get<0>(b) << ", " << get<1>(b) << ", " << get<2>(b) << ", " << get<3>(b) << ")";
    }

    template<fixed_string SourceElements>
    [[nodiscard]] static native_simd swizzle_numbers() noexcept
    {
        constexpr auto one_mask = detail::native_swizzle_to_mask<SourceElements, size, '1'>();
        constexpr auto zero_mask = detail::native_swizzle_to_mask<SourceElements, size, '0'>();
        constexpr auto number_mask = one_mask | zero_mask;
        constexpr auto alpha_mask = ~number_mask & 0b1111;

        if constexpr ((zero_mask | alpha_mask) == 0b1111) {
            return {};

        } else if constexpr ((one_mask | alpha_mask) == 0b1111) {
            return broadcast(1.0f);

        } else {
            return native_simd{
                to_bool(one_mask & 0b0001) ? 1.0f : 0.0f,
                to_bool(one_mask & 0b0010) ? 1.0f : 0.0f,
                to_bool(one_mask & 0b0100) ? 1.0f : 0.0f,
                to_bool(one_mask & 0b1000) ? 1.0f : 0.0f};
        }
    }
};

#endif

}} // namespace hi::v1
