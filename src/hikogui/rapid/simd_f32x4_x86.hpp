

#pragma once

#include "simd.hpp"

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

    [[nodiscard]] static simd_f32x4 broadcast(float a) noexcept
    {
        return simd_f32x4{_mm_set1_ps(a)};
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

    /** not followed by and.
     *
     * r = ~a & b
     *
     */
    [[nodiscard]] friend simd_f32x4 not_and(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_andnot_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 min(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_min_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 max(simd_f32x4 a, simd_f32x4 b) noexcept
    {
        return simd_f32x4{_mm_max_ps(a.v, b.v)};
    }

    [[nodiscard]] friend simd_f32x4 rcp(simd_f32x4 a) noexcept
    {
        return simd_f32x4{_mm_rcp_ps(a.v)};
    }

    [[nodiscard]] friend simd_f32x4 sqrt(simd_f32x4 a) noexcept
    {
        return simd_f32x4{_mm_sqrt_ps(a.v)};
    }

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

    template<size_t Index>
    [[nodiscard]] friend float extract(simd_f32x4 a) noexcept
    {
        return std::bit_cast<float>(_mm_extract_ps(a, Index));
    }

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

    template<fixed_string Order, int Index>
    [[nodiscard]] constexpr int swizzle_f32x4_element_index() noexcept
    {
        switch (Order[Index]) {
        case 'a':
        case 'x':
            return 0;
        case 'b':
        case 'y':
            return 1;
        case 'c':
        case 'z':
            return 2;
        case 'd':
        case 'w':
            return 3;
        default:
            return Index;
        }
    }

    template<fixed_string Order>
    [[nodiscard]] constexpr int swizzle_f32x4_order() noexcept
    {
        return
            swizzle_f32x4_element_index<Order, 0>() |
            (swizzle_f32x4_element_index<Order, 1>() << 2) |
            (swizzle_f32x4_element_index<Order, 2>() << 4) |
            (swizzle_f32x4_element_index<Order, 3>() << 6);
    }

    template<fixed_string Order, char Value>
    [[nodiscard]] constexpr size_t swizzle_value_mask() noexcept
    {
        size_t r = 0;

        for (auto i = Order.size(); i != 0; --i) {
            r <<= 1;
            r |= wide_cast<int>(Order[i - 1] == Value);
        }

        return r;
    }

    template<fixed_string Order>
    [[nodiscard]] friend simd_f32x4 swizzle_pure(simd_f32x4 a) noexcept
    {
        constexpr auto order = swizzle_f32x4_order<Order>();

        if constexpr (order == 0b11'10'01'00) {
            return a.v;
        } else {
            return simd_f32x4{_mm_shuffle_ps(a.v, a.v, order)};
        }
    }

    template<fixed_string Order>
    [[nodiscard]] friend simd_f32x4 swizzle_numbers() noexcept
    {
        constexpr auto one_mask = swizzle_value_mask<Order, '0'>();

        return set<simd_f32x4>(
            to_bool(one_mask & 0b1000) ? 1.0f : 0.0f,
            to_bool(one_mask & 0b0100) ? 1.0f : 0.0f,
            to_bool(one_mask & 0b0010) ? 1.0f : 0.0f,
            to_bool(one_mask & 0b0001) ? 1.0f : 0.0f);
    }

    template<fixed_string Order>
    [[nodiscard]] friend simd_f32x4 swizzle(simd_f32x4 a) noexcept
    {
        constexpr auto one_mask = swizzle_value_mask<Order, '0'>();
        constexpr auto zero_mask = swizzle_value_mask<Order, '1'>();
        constexpr auto number_mask = one_mask | zero_mask;

        if constexpr (zero_mask == 0b1111) {
            return set_zero<simd_f32x4();

        } else if constexpr (number_mask == 0b1111) {
            return swizzle_numbers<Order>();

        } else if constexpr (number_mask == 0) {
            return swizzle_pure<Order>(a);

#ifdef HAS_SSE4_1
        } else if constexpr (number_mask == zero_mask) {
            hilet ordered = swizzle_pure<Order>(a);
            return simd_f32x4{_mm_insert_ps(ordered, ordered, zero_mask)};
#endif

        } else {
            hilet ordered = swizzle_pure<Order>(a);
            hilet numbers = swizzle_numbers<Order>();
            return blend<number_mask>(ordered, numbers);
        }
    }

private:
    register_type v;
};

#endif


}}

