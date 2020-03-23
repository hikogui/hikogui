// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/vec.hpp"
#include <fmt/format.h>
#include <xmmintrin.h>
#include <immintrin.h>
#include <smmintrin.h>
#include <emmintrin.h>
#include <cstdint>
#include <stdexcept>
#include <array>
#include <type_traits>
#include <ostream>

namespace TTauri {

/** A 4D vector.
 *
 * If you need a 3D vector or point, you can use this vector class
 * as a homogenious coordinate.
 *
 *
 * This class supports swizzeling. Swizzeling is done using member functions which
 * will return a `vec`. The name of the member function consists of 2 to 4 of the
 * following characters: 'x', 'y', 'z', 'w', 'r', 'g', 'b', 'a', '0' & '1'.
 * If the swizzle member function name would start with a '0' or '1' character it
 * will be prefixed with an underscore '_'.
 *
 * Since swizzle member functions always return a 4D vec, the third and forth
 * element will default to '0' and 'w'. This allows a 2D vector to maintain its
 * homogeniousness.
 */ 
class ivec {
    /* Intrinsic value of the vec.
     * The elements in __m128i are assigned as follows.
     *  - [127:96] w
     *  - [95:64] z
     *  - [63:32] y
     *  - [31:0] x
     */
    __m128i v;

public:
    /* Create a zeroed out vec.
     */
    force_inline ivec() noexcept : ivec(_mm_setzero_si128()) {}
    force_inline ivec(ivec const &rhs) = default;
    force_inline ivec &operator=(ivec const &rhs) = default;
    force_inline ivec(ivec &&rhs) = default;
    force_inline ivec &operator=(ivec &&rhs) = default;

    /** Create a ivec out of a __m128i
     */
    force_inline ivec(__m128i rhs) noexcept :
        v(rhs) {}

    /** Create a ivec out of a __m128i
     */
    force_inline ivec &operator=(__m128i rhs) noexcept {
        v = rhs;
        return *this;
    }

    /** Convert a ivec to a __m128i.
     */
    force_inline operator __m128i () const noexcept {
        return v;
    }

    force_inline ivec(vec const &rhs) noexcept :
       ivec(_mm_cvtps_epi32(rhs)) {}

    force_inline ivec &operator=(vec const &rhs) noexcept {
        return *this = _mm_cvtps_epi32(rhs);
    }

    force_inline operator vec () const noexcept {
        return _mm_cvtepi32_ps(*this);
    }

    explicit force_inline operator std::array<int32_t,4> () const noexcept {
        std::array<int32_t,4> r;
        _mm_storeu_si128(reinterpret_cast<__m128i*>(r.data()), *this);
        return r;
    }

    /** Initialize a ivec with all elements set to a value.
     * Useful as a scalar converter, when combined with an
     * arithmetic operator.
     */
    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    explicit force_inline ivec(T rhs) noexcept:
        ivec(_mm_set1_epi32(numeric_cast<int32_t>(rhs))) {}

    /** Initialize a ivec with all elements set to a value.
     * Useful as a scalar converter, when combined with an
     * arithmetic operator.
     */
    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    force_inline ivec &operator=(T rhs) noexcept {
        return *this = _mm_set1_epi32(numeric_cast<int32_t>(rhs));
    }

    /** Create a ivec out of 2 to 4 values.
    * This vector is used as a homogeneous coordinate, meaning:
    *  - vectors have w=0 (A direction and distance)
    *  - points have w=1 (A position in space)
    */
    template<typename T, typename U, typename V=int, typename W=int,
        std::enable_if_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U> && std::is_arithmetic_v<V> && std::is_arithmetic_v<W>,int> = 0>
    force_inline ivec(T x, U y, V z=0, W w=0) noexcept :
        ivec(_mm_set_epi32(
            numeric_cast<int32_t>(w),
            numeric_cast<int32_t>(z),
            numeric_cast<int32_t>(y),
            numeric_cast<int32_t>(x)
            )) {}

    /** Create a ivec out of 2 to 4 values.
    * This vector is used as a homogeneous coordinate, meaning:
    *  - vectors have w=0 (A direction and distance)
    *  - points have w=1 (A position in space)
    */
    template<typename T, typename U, typename V=int, typename W=int,
    std::enable_if_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<U> && std::is_arithmetic_v<V> && std::is_arithmetic_v<W>,int> = 0>
    [[nodiscard]] force_inline static ivec point(T x, U y, V z=0, W w=1) noexcept {
        return ivec(x, y, z, w);
    }

    template<size_t I, typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    force_inline ivec &set(T rhs) noexcept {
        static_assert(I <= 3);
        return *this = _mm_insert_epi32(*this, numeric_cast<int32_t>(rhs), I);
    }

    template<size_t I>
    force_inline int get() const noexcept {
        static_assert(I <= 3);
        return _mm_extract_epi32(*this, I);
    }

    constexpr size_t size() const noexcept { return 4; }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    force_inline ivec &x(T rhs) noexcept { return set<0>(rhs); }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    force_inline ivec &y(T rhs) noexcept { return set<1>(rhs); }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    force_inline ivec &z(T rhs) noexcept { return set<2>(rhs); }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    force_inline ivec &w(T rhs) noexcept { return set<3>(rhs); }

    force_inline int x() const noexcept { return get<0>(); }
    force_inline int y() const noexcept { return get<1>(); }
    force_inline int z() const noexcept { return get<2>(); }
    force_inline int w() const noexcept { return get<3>(); }

    force_inline ivec &operator+=(ivec const &rhs) noexcept {
        return *this = _mm_add_epi32(*this, rhs);
    }

    force_inline ivec &operator-=(ivec const &rhs) noexcept {
        return *this = _mm_sub_epi32(*this, rhs);
    }

    force_inline ivec &operator*=(ivec const &rhs) noexcept {
        return *this = _mm_mullo_epi32(*this, rhs);
    }


    [[nodiscard]] force_inline friend ivec operator+(ivec const &lhs, ivec const &rhs) noexcept {
        return _mm_add_epi32(lhs, rhs);
    }

    [[nodiscard]] force_inline friend ivec operator-(ivec const &lhs, ivec const &rhs) noexcept {
        return _mm_sub_epi32(lhs, rhs);
    }

    [[nodiscard]] force_inline friend ivec operator*(ivec const &lhs, ivec const &rhs) noexcept {
        return _mm_mullo_epi32(lhs, rhs);
    }

    [[nodiscard]] force_inline friend ivec max(ivec const &lhs, ivec const &rhs) noexcept {
        return _mm_max_epi32(lhs, rhs);
    }

    [[nodiscard]] force_inline friend ivec min(ivec const &lhs, ivec const &rhs) noexcept {
        return _mm_min_epi32(lhs, rhs);
    }

    [[nodiscard]] force_inline friend bool operator==(ivec const &lhs, ivec const &rhs) noexcept {
        let tmp2 = _mm_movemask_epi8(_mm_cmpeq_epi32(lhs, rhs));
        return tmp2 == 0xffff;
    }

    [[nodiscard]] force_inline friend bool operator!=(ivec const &lhs, ivec const &rhs) noexcept {
        return !(lhs == rhs);
    }

    /** Equal to.
    * @return boolean nibble field, bit [3:0]=x, [7:4]=y, [11:8]=z, [15:12]=w.
    */
    [[nodiscard]] force_inline friend int eq(ivec const &lhs, ivec const &rhs) noexcept {
        return _mm_movemask_epi8(_mm_cmpeq_epi32(lhs, rhs));
    }

    /** Less than.
    * @return boolean nibble field, bit [3:0]=x, [7:4]=y, [11:8]=z, [15:12]=w.
    */
    [[nodiscard]] force_inline friend int operator<(ivec const &lhs, ivec const &rhs) noexcept {
        return _mm_movemask_epi8(_mm_cmplt_epi32(lhs, rhs));
    }

    /** Greater than.
    * @return boolean nibble field, bit [3:0]=x, [7:4]=y, [11:8]=z, [15:12]=w.
    */
    [[nodiscard]] force_inline friend int operator>(ivec const &lhs, ivec const &rhs) noexcept {
        return _mm_movemask_epi8(_mm_cmpgt_epi32(lhs, rhs));
    }

    [[nodiscard]] force_inline friend int operator<=(ivec const &lhs, ivec const &rhs) noexcept {
        return (~(lhs > rhs)) & 0xffff;
    }

    [[nodiscard]] force_inline friend int operator>=(ivec const &lhs, ivec const &rhs) noexcept {
        return (~(lhs < rhs)) & 0xffff;
    }

    [[nodiscard]] friend std::string to_string(ivec const &rhs) noexcept {
        return fmt::format("({}, {}, {}, {})", rhs.x(), rhs.y(), rhs.z(), rhs.w());
    }

    std::ostream friend &operator<<(std::ostream &lhs, ivec const &rhs) noexcept {
        return lhs << to_string(rhs);
    }

    template<std::size_t I>
    [[nodiscard]] force_inline friend int get(ivec const &rhs) noexcept {
        return rhs.get<I>();
    }

    template<char a, char b, char c, char d>
    [[nodiscard]] constexpr static int swizzle_permute_mask() noexcept {
        int r = 0;
        switch (a) {
        case 'x': r |= 0b00'00'00'00; break;
        case 'y': r |= 0b00'00'00'01; break;
        case 'z': r |= 0b00'00'00'10; break;
        case 'w': r |= 0b00'00'00'11; break;
        case '0': r |= 0b00'00'00'00; break;
        case '1': r |= 0b00'00'00'00; break;
        }
        switch (b) {
        case 'x': r |= 0b00'00'00'00; break;
        case 'y': r |= 0b00'00'01'00; break;
        case 'z': r |= 0b00'00'10'00; break;
        case 'w': r |= 0b00'00'11'00; break;
        case '0': r |= 0b00'00'01'00; break;
        case '1': r |= 0b00'00'01'00; break;
        }
        switch (c) {
        case 'x': r |= 0b00'00'00'00; break;
        case 'y': r |= 0b00'01'00'00; break;
        case 'z': r |= 0b00'10'00'00; break;
        case 'w': r |= 0b00'11'00'00; break;
        case '0': r |= 0b00'10'00'00; break;
        case '1': r |= 0b00'10'00'00; break;
        }
        switch (d) {
        case 'x': r |= 0b00'00'00'00; break;
        case 'y': r |= 0b01'00'00'00; break;
        case 'z': r |= 0b10'00'00'00; break;
        case 'w': r |= 0b11'00'00'00; break;
        case '0': r |= 0b11'00'00'00; break;
        case '1': r |= 0b11'00'00'00; break;
        }
        return r;
    }

    template<char a, char b, char c, char d>
    [[nodiscard]] force_inline ivec swizzle() const noexcept {
        constexpr int permute_mask = vec::swizzle_permute_mask<a,b,c,d>();

        __m128i swizzled;
        // Clang is able to optimize these intrinsics, MSVC is not.
        if constexpr (permute_mask != 0b11'10'01'00) {
            swizzled = _mm_shuffle_epi32(*this, permute_mask);
        } else {
            swizzled = *this;
        }

        if constexpr (a == '0' || a == '1') {
            swizzled = _mm_insert_epi32(swizzled, a == '0' ? 0 : 1, 0);
        }
        if constexpr (b == '0' || b == '1') {
            swizzled = _mm_insert_epi32(swizzled, b == '0' ? 0 : 1, 1);
        }
        if constexpr (c == '0' || c == '1') {
            swizzled = _mm_insert_epi32(swizzled, c == '0' ? 0 : 1, 2);
        }
        if constexpr (d == '0' || d == '1') {
            swizzled = _mm_insert_epi32(swizzled, d == '0' ? 0 : 1, 3);
        }

        return swizzled;
    }

#define SWIZZLE4(name, A, B, C, D)\
    [[nodiscard]] ivec name() const noexcept {\
        return swizzle<A, B, C, D>();\
    }

#define SWIZZLE4_GEN3(name, A, B, C)\
    SWIZZLE4(name ## 0, A, B, C, '0')\
    SWIZZLE4(name ## 1, A, B, C, '1')\
    SWIZZLE4(name ## x, A, B, C, 'x')\
    SWIZZLE4(name ## y, A, B, C, 'y')\
    SWIZZLE4(name ## z, A, B, C, 'z')\
    SWIZZLE4(name ## w, A, B, C, 'w')

#define SWIZZLE4_GEN2(name, A, B)\
    SWIZZLE4_GEN3(name ## 0, A, B, '0')\
    SWIZZLE4_GEN3(name ## 1, A, B, '1')\
    SWIZZLE4_GEN3(name ## x, A, B, 'x')\
    SWIZZLE4_GEN3(name ## y, A, B, 'y')\
    SWIZZLE4_GEN3(name ## z, A, B, 'z')\
    SWIZZLE4_GEN3(name ## w, A, B, 'w')

#define SWIZZLE4_GEN1(name, A)\
    SWIZZLE4_GEN2(name ## 0, A, '0')\
    SWIZZLE4_GEN2(name ## 1, A, '1')\
    SWIZZLE4_GEN2(name ## x, A, 'x')\
    SWIZZLE4_GEN2(name ## y, A, 'y')\
    SWIZZLE4_GEN2(name ## z, A, 'z')\
    SWIZZLE4_GEN2(name ## w, A, 'w')

    SWIZZLE4_GEN1(_0, '0')
    SWIZZLE4_GEN1(_1, '1')
    SWIZZLE4_GEN1(x, 'x')
    SWIZZLE4_GEN1(y, 'y')
    SWIZZLE4_GEN1(z, 'z')
    SWIZZLE4_GEN1(w, 'w')

#define SWIZZLE3(name, A, B, C)\
    [[nodiscard]] ivec name() const noexcept {\
        return swizzle<A,B,C,'w'>();\
    }

#define SWIZZLE3_GEN2(name, A, B)\
    SWIZZLE3(name ## 0, A, B, '0')\
    SWIZZLE3(name ## 1, A, B, '1')\
    SWIZZLE3(name ## x, A, B, 'x')\
    SWIZZLE3(name ## y, A, B, 'y')\
    SWIZZLE3(name ## z, A, B, 'z')\
    SWIZZLE3(name ## w, A, B, 'w')

#define SWIZZLE3_GEN1(name, A)\
    SWIZZLE3_GEN2(name ## 0, A, '0')\
    SWIZZLE3_GEN2(name ## 1, A, '1')\
    SWIZZLE3_GEN2(name ## x, A, 'x')\
    SWIZZLE3_GEN2(name ## y, A, 'y')\
    SWIZZLE3_GEN2(name ## z, A, 'z')\
    SWIZZLE3_GEN2(name ## w, A, 'w')

    SWIZZLE3_GEN1(_0, '0')
    SWIZZLE3_GEN1(_1, '1')
    SWIZZLE3_GEN1(x, 'x')
    SWIZZLE3_GEN1(y, 'y')
    SWIZZLE3_GEN1(z, 'z')
    SWIZZLE3_GEN1(w, 'w')

#define SWIZZLE2(name, A, B)\
    [[nodiscard]] ivec name() const noexcept {\
        return swizzle<A,B,'0','w'>();\
    }

#define SWIZZLE2_GEN1(name, A)\
    SWIZZLE2(name ## 0, A, '0')\
    SWIZZLE2(name ## 1, A, '1')\
    SWIZZLE2(name ## x, A, 'x')\
    SWIZZLE2(name ## y, A, 'y')\
    SWIZZLE2(name ## z, A, 'z')\
    SWIZZLE2(name ## w, A, 'w')

    SWIZZLE2_GEN1(_0, '0')
    SWIZZLE2_GEN1(_1, '1')
    SWIZZLE2_GEN1(x, 'x')
    SWIZZLE2_GEN1(y, 'y')
    SWIZZLE2_GEN1(z, 'z')
    SWIZZLE2_GEN1(w, 'w')
};

}

#undef SWIZZLE4
#undef SWIZZLE4_GEN1
#undef SWIZZLE4_GEN2
#undef SWIZZLE4_GEN3
#undef SWIZZLE3
#undef SWIZZLE3_GEN1
#undef SWIZZLE3_GEN2
#undef SWIZZLE2
#undef SWIZZLE2_GEN1
