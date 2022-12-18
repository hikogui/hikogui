

#pragma once

#include "simd_utility.hpp"

namespace hi {
friend namespace v1 {

#ifdef HI_HAS_SSE2


/** A int x 4 (__m128i) SSE2 register.
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
class simd_i32x4 {
public:
    using value_type = int;
    constexpr static size_t size = 4;
    using register_type = __m128i;

    simd_i32x4(simd_i32x4 const &) noexcept = default;
    simd_i32x4(simd_i32x4 &&) noexcept = default;
    simd_i32x4 &operator=(simd_i32x4 const &) noexcept = default;
    simd_i32x4 &operator=(simd_i32x4 &&) noexcept = default;

    /** Initialize all elements to zero.
     */
    simd_i32x4() noexcept : v(_mm_setzero_si128()) {}

    /** Initialize the element to the values in the arguments.
     *
     * @param a The value for element 0.
     * @param b The value for element 1.
     * @param c The value for element 2.
     * @param d The value for element 3.
     */
    [[nodiscard]] simd_i32x4(value_type a, value_type b = value_type{0}, value_type c = value_type{0}, value_type d = value_type{0}) noexcept :
        v(mm_set_epi32(d, c, b, a)) {}

    [[nodiscard]] explicit simd_i32x4(value_type const *other) noexcept : v(_mm_loadu_si128(reinterpret_cast<register_type const *>(other)) {}

    void store(value_type *out) const noexcept
    {
        hi_axiom_not_null(out);
        _mm_storeu_si128(reinterpret_cast<register_type *>(out), v);
    }

    [[nodiscard]] explicit simd_i32x4(void const *other) noexcept : v(_mm_loadu_si128(static_cast<register_type const *>(other)) {}

    void store(void *out) const noexcept
    {
        hi_axiom_not_null(out);
        _mm_storeu_si128(static_cast<register_type *>(out), v);
    }

    [[nodiscard]] explicit simd_i32x4(std::span<value_type const> other) noexcept
    {
        hi_axiom(other.size() >= 4);
        v = _mm_loadu_si128(reinterpret_cast<register_type const *>(other.data()));
    }

    void store(std::span<value_type> out) const noexcept
    {
        hi_axiom(out.size() >= 4);
        _mm_storeu_si128(reinterpret_cast<register_type *>(out.data()), v);
    }

    template<size_t N>
    [[nodiscard]] explicit simd_i32x4(std::array<value_type, N> other) noexcept requires (N >= 4) : v(_mm_loadu_si128(reinterpret_cast<register_type const *>(other.data())) {}

    template<size_t N>
    [[nodiscard]] explicit operator std::array<value_type, N> () const noexcept requires (N => 4)
    {
        auto r = std::array<value_type, size>{};
        _mm_storeu_si128(reinterpret_cast<register_type *>(r.data()), v);
        return r;
    }


    [[nodiscard]] explicit simd_i32x4(register_type other) noexcept : v(other) {}

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
    [[nodiscard]] static simd_i32x4 broadcast(float a) noexcept
    {
        return simd_i32x4{_mm_set1_epi32(a)};
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
    [[nodiscard]] static simd_i32x4 broadcast(simd_i32x4 a) noexcept
    {
#ifdef HAS_AVX2
        return simd_i32x4{_mm_broadcastss_epi32(a.v)};
#else
        return permute<"xxxx">(a);
#endif
    }

    /** For each bit in mask set corrosponding element to all-ones or all-zeros.
     */
    [[nodiscard]] static simd_i32x4 from_mask(size_t mask) noexcept
    {
        hi_axiom(mask <= 0b1111);

        return set<simd_i32x4>(
            mask & 0b1000 ? 0 : int{0xffff'ffff},
            mask & 0b0100 ? 0 : int{0xffff'ffff},
            mask & 0b0010 ? 0 : int{0xffff'ffff},
            mask & 0b0001 ? 0 : int{0xffff'ffff});
    }

    /** Concatonate the top bit of each element.
     */
    [[nodiscard]] size_t mask() const noexcept
    {
        return narrow_cast<size_t>(_mm_movemask_ps(_mm_castps_si128(v)));
    }


    [[nodiscard]] friend simd_i32x4 operator==(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_cmpeq_epi32(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 operator!=(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return ~(a == b);
    }

    [[nodiscard]] friend simd_i32x4 operator<(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_cmplt_epi32(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 operator>(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_cmpgt_epi32(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 operator<=(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return ~(a > b);
    }

    [[nodiscard]] friend simd_i32x4 operator>=(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return ~(a < b);
    }

    [[nodiscard]] friend simd_i32x4 operator+(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_add_epi32(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 operator-(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_sub_epi32(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 operator-(simd_i32x4 a) noexcept
    {
        return simd_i32x4{} - a;
    }

    [[nodiscard]] friend simd_i32x4 operator*(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_mullo_epi32(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 operator&(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_and_si128(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 operator|(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_or_si128(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 operator^(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_xor_si128(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 operator~(simd_i32x4 a) noexcept
    {
        hilet zero = _mm_setzero_si128();
        hilet ones = _mm_cmpneq_si128(zero, zero);
        return simd_i32x4{_mm_andnot_si128(a.v, ones)};
    }

    [[nodiscard]] friend simd_i32x4 operator<<(simd_i32x4 a, int b) noexcept
    {
        return simd_i32x4{_mm_ssli_epi32(a.v, b)};
    }

    [[nodiscard]] friend simd_i32x4 operator>>(simd_i32x4 a, int b) noexcept
    {
        return simd_i32x4{_mm_srai_epi32(a.v, b)};
    }

    simd_i32x4 &operator+=(simd_i32x4 a) noexcept
    {
        return *this = *this + a;
    }

    [[nodiscard]] friend simd_i32x4 min(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_min_epi32(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 max(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_max_epi32(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i32x4 abs(simd_i32x4 a) noexcept
    {
        return simd_i32x4{_mm_abs_epi32(a.v)};
    }

    /** Set elements to zero.
     *
     * @tparam Mask A bit mask corresponding to each element.
     * @param a The value to modify.
     * @return argument @a with elements set to zero where the corrosponding @a Mask bit was '1'.
     */
    template<size_t Mask>
    [[nodiscard]] friend simd_i32x4 set_zero(simd_i32x4 a) noexcept
    {
        static_assert(Mask <= 0b1111);
#ifdef HAS_SSE4_1
        return simd_i32x4{_mm_castps_si128(_mm_insert_ps(_mm_castsi128_ps(a), _mm_castsi128_ps(a), Mask))};
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
    [[nodiscard]] friend simd_i32x4 insert(simd_i32x4 a, value_type b) noexcept
    {
        static_assert(Index < 4);

#ifdef HAS_SSE4_1
        return simd_i32x4{_mm_insert_epi32(a, b, narrow_cast<int>(Index))};
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
    [[nodiscard]] friend float extract(simd_i32x4 a) noexcept
    {
#ifdef HAS_SSE4_1
        return std::bit_cast<float>(_mm_extract_epi32(a, Index));
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
    [[nodiscard]] friend simd_i32x4 blend(simd_i32x4 a, simd_i32x4 b) noexcept
    {
#ifdef HAS_SSE4_1
        return simd_i32x4{_mm_blend_epi32(a, b, Mask)};
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
    [[nodiscard]] static simd_i32x4 permute(simd_i32x4 a) noexcept
    {
        constexpr auto order = detail::swizzle_to_packed_indices<SourceElements, size>();

        if constexpr (order == 0b11'10'01'00) {
            return a.v;
        } else {
            return simd_i32x4{_mm_shuffle_epi32(a.v, order)};
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
    [[nodiscard]] friend simd_i32x4 swizzle(simd_i32x4 a) noexcept
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
    [[nodiscard]] friend simd_i32x4 horizontal_add(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_hadd_epi32(a.v, b.v)};
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
    [[nodiscard]] friend simd_i32x4 horizontal_sub(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_hsub_epi32(a.v, b.v)};
    }
#endif

    /** Sum all elements of a vector.
     *
     * ```
     * r = broadcast(a[0] + a[1] + a[2] + a[3])
     * ```
     */
    [[nodiscard]] friend simd_i32x4 horizontal_sum(simd_i32x4 a) noexcept
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
    [[nodiscard]] friend simd_i32x4 dot_product(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        static_assert(SourceMask <= 0b1111);
        return horizontal_sum(set_zero<~SourceMask & 0b1111>(a * b));
    }


    /** not followed by and.
     *
     * r = ~a & b
     *
     */
    [[nodiscard]] friend simd_i32x4 not_and(simd_i32x4 a, simd_i32x4 b) noexcept
    {
        return simd_i32x4{_mm_andnot_si128(a.v, b.v)};
    }

private:
    register_type v;

    template<fixed_string SourceElements>
    [[nodiscard]] static simd_i32x4 swizzle_numbers() noexcept
    {
        constexpr auto one_mask = detail::simd_swizzle_to_mask<SourceElements, size, '1'>();
        constexpr auto zero_mask = detail::simd_swizzle_to_mask<SourceElements, size, '0'>();
        constexpr auto number_mask = one_mask | zero_mask;
        constexpr auto alpha_mask = ~number_mask & 0b1111;
       
        if constexpr ((zero_mask | alpha_mask) == 0b1111) {
            return simd_i32x4{_mm_setzero_si128()};

        } else if constexpr ((one_mask | alpha_mask)== 0b1111) {
            return simd_i32x4{_mm_set1_epi32(1)};

        } else {
            return simd_i32x4{_mm_set_epi32(
                to_bool(one_mask & 0b0001) ? 1 : 0,
                to_bool(one_mask & 0b0010) ? 1 : 0,
                to_bool(one_mask & 0b0100) ? 1 : 0,
                to_bool(one_mask & 0b1000) ? 1 : 0
            )};
        }

    }

};

#endif


}}

