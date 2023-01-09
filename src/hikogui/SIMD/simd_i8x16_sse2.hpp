// Copyright Take Vos 2022, 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd_utility.hpp"
#include "../assert.hpp"
#include <array>
#include <ostream>

namespace hi { inline namespace v1 {

#ifdef HI_HAS_SSE2

/** A int8_t x 4 (__m128i) SSE2 register.
 *
 *
 * When loading and storing from memory this is the order of the element in the register
 *
 * ```
 *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *  | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |10 |11 |12 |13 |14 |15 |
 *  +---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
 *    0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15   memory address.
 * ```
 *
 * In the function below a `mask` values least-significant-bit corresponds to element 0.
 *
 */
struct simd_i8x16 {
    using value_type = int8_t;
    constexpr static size_t size = 4;
    using register_type = __m128i;
    using array_type = std::array<value_type, size>;

    register_type v;

    simd_i8x16(simd_i8x16 const&) noexcept = default;
    simd_i8x16(simd_i8x16&&) noexcept = default;
    simd_i8x16& operator=(simd_i8x16 const&) noexcept = default;
    simd_i8x16& operator=(simd_i8x16&&) noexcept = default;

    /** Initialize all elements to zero.
     */
    simd_i8x16() noexcept : v(_mm_setzero_si128()) {}

    [[nodiscard]] explicit simd_i8x16(register_type other) noexcept : v(other) {}

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
     * @param e The value for element 4.
     * @param f The value for element 5.
     * @param g The value for element 6.
     * @param h The value for element 7.
     * @param i The value for element 8.
     * @param j The value for element 9.
     * @param k The value for element 10.
     * @param l The value for element 11.
     * @param m The value for element 12.
     * @param n The value for element 13.
     * @param o The value for element 14.
     * @param p The value for element 15.
     */
    [[nodiscard]] simd_i8x16(
        value_type a,
        value_type b = value_type{0},
        value_type c = value_type{0},
        value_type d = value_type{0},
        value_type e = value_type{0},
        value_type f = value_type{0},
        value_type g = value_type{0},
        value_type h = value_type{0},
        value_type i = value_type{0},
        value_type j = value_type{0},
        value_type k = value_type{0},
        value_type l = value_type{0},
        value_type m = value_type{0},
        value_type n = value_type{0},
        value_type o = value_type{0},
        value_type p = value_type{0},
        ) noexcept :
        v(_mm_set_epi8(p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a))
    {
    }

    [[nodiscard]] explicit simd_i8x16(value_type const *other) noexcept :
        v(_mm_loadu_si128(reinterpret_cast<register_type const *>(other)))
    {
    }

    void store(value_type *out) const noexcept
    {
        hi_axiom_not_null(out);
        _mm_storeu_si128(reinterpret_cast<register_type *>(out), v);
    }

    [[nodiscard]] explicit simd_i8x16(void const *other) noexcept : v(_mm_loadu_si128(static_cast<register_type const *>(other)))
    {
    }

    void store(void *out) const noexcept
    {
        hi_axiom_not_null(out);
        _mm_storeu_si128(static_cast<register_type *>(out), v);
    }

    [[nodiscard]] explicit simd_i8x16(std::span<value_type const> other) noexcept
    {
        hi_axiom(other.size() >= size);
        v = _mm_loadu_si128(reinterpret_cast<register_type const *>(other.data()));
    }

    void store(std::span<value_type> out) const noexcept
    {
        hi_axiom(out.size() >= size);
        _mm_storeu_si128(reinterpret_cast<register_type *>(out.data()), v);
    }

    [[nodiscard]] explicit simd_i8x16(array_type other) noexcept :
        v(_mm_loadu_si128(reinterpret_cast<register_type const *>(other.data())))
    {
    }

    [[nodiscard]] explicit operator array_type() const noexcept
    {
        auto r = array_type{};
        _mm_storeu_si128(reinterpret_cast<register_type *>(r.data()), v);
        return r;
    }

#ifdef AVX512F
    [[nodiscard]] explicit simd_i8x16(simd_f32x16 const& a) noexcept;
    [[nodiscard]] explicit simd_i8x16(simd_u32x16 const& a) noexcept;
#endif

    /** Broadcast a single value to all the elements.
     *
     * ```
     * r[ 0] = a; r[ 1] = a; r[ 2] = a; r[ 3] = a;
     * r[ 4] = a; r[ 5] = a; r[ 6] = a; r[ 7] = a;
     * r[ 8] = a; r[ 9] = a; r[10] = a; r[11] = a;
     * r[12] = a; r[13] = a; r[14] = a; r[15] = a;
     * ```
     */
    [[nodiscard]] static simd_i8x16 broadcast(value_type a) noexcept
    {
        return simd_i8x16{_mm_set1_epi8(a)};
    }

    /** Broadcast the first element to all the elements.
     *
     * ```
     * r[ 0] = a[0]
     * r[ 1] = a[0]
     * r[ 2] = a[0]
     * r[ 3] = a[0]
     * r[ 4] = a[0]
     * r[ 5] = a[0]
     * r[ 6] = a[0]
     * r[ 7] = a[0]
     * r[ 8] = a[0]
     * r[ 9] = a[0]
     * r[10] = a[0]
     * r[11] = a[0]
     * r[12] = a[0]
     * r[13] = a[0]
     * r[14] = a[0]
     * r[15] = a[0]
     * ```
     */
    [[nodiscard]] static simd_i8x16 broadcast(simd_i8x16 a) noexcept
    {
#ifdef HI_HAS_AVX2
        return simd_i8x16{_mm_broadcastb_epi8(a.v)};
#elif HI_HAS_SSSE3
        return simd_i8x16{_mm_shuffle_epi8(a.v, _mm_setzero_si128())};
#else
        auto tmp = _mm_extract_epi16(a.v, 0) & 0xff;
        tmp <<= 8;
        tmp |= tmp;
        tmp <<= 16;
        tmp |= tmp;
        return simd_i8x16{_mm_shuffle_epi8(tmp, 0b00'00'00'00)};
#endif
    }

    [[nodiscard]] static simd_i8x16 ones() noexcept
    {
        return simd_i8x16{_mm_castps_si128(_mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps()))};
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return *this == simd_i8x16{};
    }

    explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Concatenate the top bit of each element.
     */
    [[nodiscard]] size_t mask() const noexcept
    {
        return narrow_cast<size_t>(_mm_movemask_epi8(v));
    }

    [[nodiscard]] friend bool operator==(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return eq(a, b).mask() == 0b1111'1111'1111'1111;
    }

    [[nodiscard]] friend simd_i8x16 eq(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return simd_i8x16{_mm_cmpeq_epi8(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i8x16 ne(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return ~eq(a, b);
    }

    [[nodiscard]] friend simd_i8x16 lt(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return simd_i8x16{_mm_cmplt_epi8(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i8x16 gt(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return simd_i8x16{_mm_cmpgt_epi8(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i8x16 le(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return ~gt(a, b);
    }

    [[nodiscard]] friend simd_i8x16 ge(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return ~lt(a, b);
    }

    [[nodiscard]] friend simd_i8x16 operator+(simd_i8x16 a) noexcept
    {
        return a;
    }

    [[nodiscard]] friend simd_i8x16 operator-(simd_i8x16 a) noexcept
    {
        return simd_i8x16{} - a;
    }

    [[nodiscard]] friend simd_i8x16 operator+(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return simd_i8x16{_mm_add_epi8(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i8x16 operator-(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return simd_i8x16{_mm_sub_epi8(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i8x16 operator&(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return simd_i8x16{_mm_and_si128(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i8x16 operator|(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return simd_i8x16{_mm_or_si128(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i8x16 operator^(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return simd_i8x16{_mm_xor_si128(a.v, b.v)};
    }

    [[nodiscard]] friend simd_i8x16 operator~(simd_i8x16 a) noexcept
    {
        hilet ones = _mm_castps_si128(_mm_cmpeq_ps(_mm_setzero_ps(), _mm_setzero_ps()));
        return simd_i8x16{_mm_andnot_si128(a.v, ones)};
    }

    [[nodiscard]] friend simd_i8x16 min(simd_i8x16 a, simd_i8x16 b) noexcept
    {
#if HI_HAS_SSE4_1
        return simd_i8x16{_mm_min_epi8(a.v, b.v)};
#else
        hilet mask = lt(a, b);
        return (mask & a) | not_and(mask, b);
#endif
    }

    [[nodiscard]] friend simd_i8x16 max(simd_i8x16 a, simd_i8x16 b) noexcept
    {
#if HI_HAS_SSE4_1
        return simd_i8x16{_mm_max_epi8(a.v, b.v)};
#else
        hilet mask = gt(a, b);
        return (mask & a) | not_and(mask, b);
#endif
    }

    [[nodiscard]] friend simd_i8x16 abs(simd_i8x16 a) noexcept
    {
#if HI_HAS_SSSE3
        return simd_i8x16{_mm_abs_epi8(a.v)};
#else
        hilet mask = gt(a, simd_i8x16{});
        return (mask & a) | not_and(mask, -a);
#endif
    }

    /** Set elements to zero.
     *
     * @tparam Mask A bit mask corresponding to each element.
     * @param a The value to modify.
     * @return argument @a with elements set to zero where the corrosponding @a Mask bit was '1'.
     */
    template<size_t Mask>
    [[nodiscard]] friend simd_i8x16 set_zero(simd_i8x16 a) noexcept
    {
        static_assert(Mask <= 0b1111);
#ifdef HI_HAS_SSE4_1
        return simd_i8x16{_mm_castps_si128(_mm_insert_ps(_mm_castsi128_ps(a.v), _mm_castsi128_ps(a.v), Mask))};
#else
        hilet mask = from_mask<Mask>();
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
    [[nodiscard]] friend simd_i8x16 insert(simd_i8x16 a, value_type b) noexcept
    {
        static_assert(Index < 4);

#ifdef HI_HAS_SSE4_1
        return simd_i8x16{_mm_insert_epi8(a.v, b, Index)};
#else
        hilet mask = from_mask<1_uz << Index>();
        return not_and(mask, a) | (mask & broadcast(b));
#endif
    }

    /** Extract an element from a vector.
     *
     * @tparam Index the index of the element.
     * @param a The vector to select the element from.
     * @return The value of the selected element.
     */
    template<size_t Index>
    [[nodiscard]] friend value_type get(simd_i8x16 a) noexcept
    {
#ifdef HI_HAS_SSE4_1
        return _mm_extract_epi8(a.v, Index);
#else
        auto r = static_cast<array_type>(a);
        return std::get<Index>(r);
#endif
    }

    /** not followed by and.
     *
     * r = ~a & b
     *
     */
    [[nodiscard]] friend simd_i8x16 not_and(simd_i8x16 a, simd_i8x16 b) noexcept
    {
        return simd_i8x16{_mm_andnot_si128(a.v, b.v)};
    }

    friend std::ostream& operator<<(std::ostream& a, simd_i8x16 b) noexcept
    {
        return a << "(" << get<0>(b) << ", " << get<1>(b) << ", " << get<2>(b) << ", " << get<3>(b) << ")";
    }
};

template<>
struct low_level_simd<int8_t, 16> : std::true_type {
    using type = simd_i8x16;
};

#endif

}} // namespace hi::v1