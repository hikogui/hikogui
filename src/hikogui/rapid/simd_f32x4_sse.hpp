

#pragma once

#include "simd_utility.hpp"

namespace hi {
friend namespace v1 {

#ifdef HI_HAS_SSE


/** A float x 4 (__m128) SSE register.
 *
 *
 * When loading and storing from memory this is the order of the element in the register
 *
 * ```
 *   lo           hi lo           hi lo           hi lo           hi 
 *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *  | element 0/a/x | element 1/b/y | element 2/c/z | element 3/d/w |
 *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15   memory address.
 * ```
 *
 * In the function below a `mask` values least-significan-bit corrosponds to element 0.
 *
 */
class simd_f32x4 {
public:
    using value_type = float;
    constexpr static size_t size = 4;
    using register_type = __m128;

    simd_f32x4(simd_f32x4 const &) noexcept = default;
    simd_f32x4(simd_f32x4 &&) noexcept = default;
    simd_f32x4 &operator=(simd_f32x4 const &) noexcept = default;
    simd_f32x4 &operator=(simd_f32x4 &&) noexcept = default;

    /** Initialize all elements to zero.
     */
    simd_f32x4() noexcept : v(_mm_setzero_ps()) {}

    /** Initialize the first element to @a andother elements to zero.
     *
     * @param a The value for element 0.
     */
    [[nodiscard]] explicit simd_f32x4(value_type a) noexcept : v(_mm_set_ss(a)) {}

    /** Initialize the element to the values in the arguments.
     *
     * @param a The value for element 0.
     * @param b The value for element 1.
     * @param c The value for element 2.
     * @param d The value for element 3.
     */
    [[nodiscard]] simd_f32x4(value_type a, value_type b, value_type c = value_type{0}, value_type d = value_type{0}) noexcept :
        v(mm_set_ps(d, c, b, a)) {}

    [[nodiscard]] explicit simd_f32x4(value_type const *other) noexcept : v(_mm_loadu_ps(other) {}

    void store(value_type *out) const noexcept
    {
        hi_axiom_not_null(out);
        _mm_storeu_ps(out, v);
    }

    [[nodiscard]] explicit simd_f32x4(void const *other) noexcept : v(_mm_loadu_ps(static_cast<float *>(other)) {}

    void store(void *out) const noexcept
    {
        hi_axiom_not_null(out);
        _mm_storeu_ps(static_cast<float *>(out), v);
    }

    [[nodiscard]] explicit simd_f32x4(std::span<value_type const> other) noexcept
    {
        hi_axiom(other.size() >= 4);
        v = _mm_loadu_ps(other.data());
    }

    void store(std::span<value_type> out) const noexcept
    {
        hi_axiom(out.size() >= 4);
        _mm_storeu_ps(out.data(), v);
    }

    template<size_t N>
    [[nodiscard]] explicit simd_f32x4(std::array<value_type, N> other) noexcept requires (N >= 4) : v(_mm_loadu_ps(other.data()) {}

    template<size_t N>
    [[nodiscard]] explicit operator std::array<value_type, N> () const noexcept requires (N => 4)
    {
        auto r = std::array<value_type, size>{};
        _mm_storeu_ps(r.data(), v);
        return r;
    }


    [[nodiscard]] explicit simd_f32x4(register_type other) noexcept : v(other) {}

    [[nodiscard]] explicit operator register_type () const noexcept {
        return v;
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
    [[nodiscard]] static simd_f32x4 broadcast(float a) noexcept
    {
        return simd_f32x4{_mm_set1_ps(a)};
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
    [[nodiscard]] static simd_f32x4 broadcast(simd_f32x4 a) noexcept
    {
#ifdef HAS_AVX2
        return simd_f32x4{_mm_broadcastss_ps(a.v)};
#else
        return permute<"xxxx">(a);
#endif
    }

    /** For each bit in mask set corrosponding element to all-ones or all-zeros.
     */
    [[nodiscard]] static simd_f32x4 from_mask(size_t mask) noexcept
    {
        hi_axiom(mask <= 0b1111);

        constexpr auto all_ones = std::bit_cast<float>(uint32_t{0xffff'ffff});
        return set<simd_f32x4>(
            mask & 0b1000 ? 0.0f : all_ones,
            mask & 0b0100 ? 0.0f : all_ones,
            mask & 0b0010 ? 0.0f : all_ones,
            mask & 0b0001 ? 0.0f : all_ones);
    }

    /** Concatonate the top bit of each element.
     */
    [[nodiscard]] size_t mask() const noexcept
    {
        return narrow_cast<size_t>(_mm_movemask_ps(v));
    }


    [[nodiscard]] friend simd_f32x4 operator==(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_cmpeq_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator!=(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_cmpneq_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator<(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_cmplt_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator>(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_cmpgt_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator<=(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_cmple_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator>=(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_cmpge_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator+(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_add_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator-(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_sub_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator-(simd_f32x4 a) noexcept
    {
        return simd_f32x4{} - a;
    }

    [[nodiscard]] friend simd_f32x4 operator*(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_mul_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator/(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_div_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator&(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_and_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator|(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_or_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator^(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_xor_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 operator~(simd_f32x4 a) noexcept
    {
        hilet zero = _mm_setzero_ps();
        hilet ones = _mm_cmpneq_ps(zero, zero);
        return simd_f32x4{_mm_andnot_ps(a.v, ones)};
    }

    [[nodiscard]] friend simd_f32x4 min(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_min_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 max(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_max_ps(a.v, b.v)};
    }

#ifdef HAS_SSE4_1
    [[nodiscard]] friend simd_f32x4 floor(simd_f32x4 a) noexcept
    {
        return simd_f32x4{_mm_floor_ps(a.v)};
    }
#endif

#ifdef HAS_SSE4_1
    [[nodiscard]] friend simd_f32x4 ceil(simd_f32x4 a) noexcept
    {
        return simd_f32x4{_mm_ceil_ps(a.v)};
    }
#endif

#ifdef HAS_SSE4_1
    template<simd_rounding_mode Rounding = simd_rouding_mode::current>
    [[nodiscard]] friend simd_f32x4 round(simd_f32x4 a) noexcept
    {
        return simd_f32x4{_mm_round_ps(a.v, to_underlying(Rounding))};
    }
#endif

    /** Reciprocal.
     */
    [[nodiscard]] friend simd_f32x4 rcp(simd_f32x4 a) noexcept
    {
        return simd_f32x4{_mm_rcp_ps(a.v)};
    }

    /** Square root.
     */
    [[nodiscard]] friend simd_f32x4 sqrt(simd_f32x4 a) noexcept
    {
        return simd_f32x4{_mm_sqrt_ps(a.v)};
    }

    /** Reciprocal of the square root.
     *
     * This is often implemented in hardware using a much faster algorithm than
     * either the reciprocal and square root separately. But has slightly less
     * accuracy, see https://en.wikipedia.org/wiki/Fast_inverse_square_root
     */
    [[nodiscard]] friend simd_f32x4 rsqrt(simd_f32x4 a) noexcept
    {
        return simd_f32x4{_mm_rsqrt_ps(a.v)};
    }

    /** Set elements to zero.
     *
     * @tparam Mask A bit mask corresponding to each element.
     * @param a The value to modify.
     * @return argument @a with elements set to zero where the corrosponding @a Mask bit was '1'.
     */
    template<size_t Mask>
    [[nodiscard]] friend simd_f32x4 set_zero(simd_f32x4 a) noexcept
    {
        static_assert(Mask <= 0b1111);
#ifdef HAS_SSE4_1
        return simd_f32x4{_mm_insert_ps(a, a, Mask)};
#else
        hilet mask = move_mask(1_uz << Mask);
        return not_and(mask, a);
#endif
    }

    /** Insert a value into an element of a vector.
     *
     * @tparam Index the index of the element where insert the value.
     * @param a The vector to insert the value into.
     * @param b The value to insert.
     * @return The vector with the inserted value.
     */
    template<size_t Index>
    [[nodiscard]] friend simd_f32x4 insert(simd_f32x4 a, float b) noexcept
    {
        static_assert(Index < 4);

        hilet b_ = _mm_set1_ps(b);
#ifdef HAS_SSE4_1
        return simd_f32x4{_mm_insert_ps(a, b_, narrow_cast<int>(Index << 4))};
#else
        hilet mask = move_mask(1_uz << Index);
        return not_and(mask, a) | (mask & b_);
#endif
    }

    /** Extract an element from a vector.
     *
     * @tparam Index the index of the element.
     * @param a The vector to select the element from.
     * @return The value of the selected element.
     */
    template<size_t Index>
    [[nodiscard]] friend float extract(simd_f32x4 a) noexcept
    {
#ifdef HAS_SSE4_1
        return std::bit_cast<float>(_mm_extract_ps(a, Index));
#else
        auto r = static_cast<std::array<float, 4>(a);
        return std::get<Index>(r);
#endif
    }

    /** Select elements from two vectors.
     *
     * @tparam Mask A mask to select bits from @a a when '0'; or @a b when '1'. The
     *         lsb corrosponds with element zero.
     * @param a A vector for which element are selected when the bit in @a Mask is '0'.
     * @param b A vector for which element are selected when the bit in @a Mask is '1'.
     * @return A vector with element selected from @a a and @a b
     */
    template<size_t Mask>
    [[nodiscard]] friend simd_f32x4 blend(simd_f32x4 a, simd_f32x4 b) noexcept
    {
#ifdef HAS_SSE4_1
        return simd_f32x4{_mm_blend_ps(a, b, Mask)};
#else
        hilet mask = move_mask(Mask);
        return not_and(mask, a) | (mask & b);
#endif
    }

    /** Permute elements, ignoring numeric elements.
     *
     * The characters in @a SourceElements mean the following:
     * - 'a' - 'p': The indices to elements 0 and 15 of @a a.
     * - 'x', 'y', 'z', 'w'': The indices to elements 0, 1, 2, 3 of @a a.
     * - Any other character is treated as if the original element was selected.
     *
     * @tparam SourceElements A string reprecenting the order of elements. First character
     *         matches the first element.
     * @param a The vector to swizzle the elements
     * @returns A vector with the elements swizzled.
     */
    template<fixed_string SourceElements>
    [[nodiscard]] static simd_f32x4 permute(simd_f32x4 a) noexcept
    {
        constexpr auto order = detail::swizzle_to_packed_indices<SourceElements, size>();

        if constexpr (order == 0b11'10'01'00) {
            return a.v;
        } else {
#ifdef HAS_AVX
            return simd_f32x4{_mm_permute_ps(a.v, a.v, order)};
#else
            return simd_f32x4{_mm_shuffle_ps(a.v, a.v, order)};
#endif
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
     * @tparam SourceElements A string reprecenting the order of elements. First character
     *         matches the first element.
     * @param a The vector to swizzle the elements
     * @returns A vector with the elements swizzled.
     */
    template<fixed_string SourceElements>
    [[nodiscard]] friend simd_f32x4 swizzle(simd_f32x4 a) noexcept
    {
        constexpr auto one_mask = detail::simd_swizzle_to_mask<SourceElements, size, '1'>();
        constexpr auto zero_mask = detail::simd_swizzle_to_mask<SourceElements, size, '0'>();
        constexpr auto number_mask = one_mask | zero_mask;

        if constexpr (number_mask == 0b1111) {
            // Swizzle was /[01][01][01][01]/.
            return swizzle_numbers<SourceElements>();

        } else if constexpr (number_mask == 0b0000) {
            // Swizzle was /[^01][^01][^01][^01]/.
            return permute<SourceElements>(a);

#ifdef HAS_SSE4_1
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

#ifdef HAS_SSE3
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
    [[nodiscard]] friend simd_f32x4 horizontal_add(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_hadd_ps(a.v, b.v)};
    }
#endif

#ifdef HAS_SSE3
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
    [[nodiscard]] friend simd_f32x4 horizontal_sub(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_hsub_ps(a.v, b.v)};
    }
#endif

    /** Sum all elements of a vector.
     *
     * ```
     * r = broadcast(a[0] + a[1] + a[2] + a[3])
     * ```
     */
    [[nodiscard]] friend simd_f32x4 horizontal_sum(simd_f32x4 a) noexcept
    {
        auto tmp = a + permute<"cdab">(a);
        return tmp + permute<"badc">(tmp);
    }

    /** Dot product.
     *
     * ```
     * tmp[0] = SourceMask[0] ? a[0] * b[0] : 0
     * tmp[1] = SourceMask[1] ? a[1] * b[1] : 0
     * tmp[2] = SourceMask[2] ? a[2] * b[2] : 0
     * tmp[3] = SourceMask[3] ? a[3] * b[3] : 0
     * r = broadcast(tmp[0] + tmp[1] + tmp[2] + tmp[3])
     * ```
     */
    template<size_t SourceMask>
    [[nodiscard]] friend simd_f32x4 dot_product(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        static_assert(SourceMask <= 0b1111);
#ifdef HAS_SSE4_1
        return simd_f32x4{_mm_dp_ps(a.v, b.v, SourceMask << 4)};
#else
        return horizontal_sum(set_zero<~SourceMask & 0b1111>(a * b));
#endif
    }


#ifdef HAS_SSE3
    /** Interleaved subtract and add alements.
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
    [[nodiscard]] friend simd_f32x4 interleaved_sub_add(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_addsub_ps(a.v, b.v)};
    }
#endif

    /** not followed by and.
     *
     * r = ~a & b
     *
     */
    [[nodiscard]] friend simd_f32x4 not_and(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_andnot_ps(a.v, b.v)};
    }

private:
    register_type v;

    template<fixed_string SourceElements>
    [[nodiscard]] static simd_f32x4 swizzle_numbers() noexcept
    {
        constexpr auto one_mask = detail::simd_swizzle_to_mask<SourceElements, size, '1'>();
        constexpr auto zero_mask = detail::simd_swizzle_to_mask<SourceElements, size, '0'>();
        constexpr auto number_mask = one_mask | zero_mask;
        constexpr auto alpha_mask = ~number_mask & 0b1111;
       
        if constexpr ((zero_mask | alpha_mask) == 0b1111) {
            return simd_f32x4{_mm_setzero_ps()};

        } else if constexpr ((one_mask | alpha_mask)== 0b1111) {
            return simd_f32x4{_mm_set1_ps(1.0f)};

        } else {
            return simd_f32x4{_mm_set_ps(
                to_bool(one_mask & 0b0001) ? 1.0f : 0.0f,
                to_bool(one_mask & 0b0010) ? 1.0f : 0.0f,
                to_bool(one_mask & 0b0100) ? 1.0f : 0.0f,
                to_bool(one_mask & 0b1000) ? 1.0f : 0.0f
            )};
        }

    }

};

#endif


}}

