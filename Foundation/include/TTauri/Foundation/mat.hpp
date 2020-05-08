// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/rect.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <ostream>

namespace TTauri {

/** A 4x4 matrix.
 * You can use this to transform vec (which has 4 elements)
 */
class mat {
    vec col0;
    vec col1;
    vec col2;
    vec col3;

public:
    /** Create an identity matrix.
     */
    force_inline mat() noexcept {}
    force_inline mat(mat const &rhs) noexcept = default;
    force_inline mat &operator=(mat const &rhs) noexcept = default;
    force_inline mat(mat &&rhs) noexcept = default;
    force_inline mat &operator=(mat &&rhs) noexcept = default;

    /** Create a matrix for 4 vector-columns
     */
    force_inline mat(vec col0, vec col1, vec col2, vec col3) noexcept :
        col0(col0), col1(col1), col2(col2), col3(col3) {}

    /** Optimized scale matrix.
    */
    struct S {
        vec s;

        explicit S(vec rhs) noexcept :
            s(rhs) { ttauri_assume(rhs.is_point()); }

        S(float x, float y, float z=0.0f) noexcept :
            s(x, y, z, 1.0f) {}

        /** Create a scaling matrix.
        */
        operator mat () const noexcept {
            ttauri_assume(s.is_point());
            let tmp = _mm_setzero_ps();
            let col0 = _mm_insert_ps(tmp, s, 0b00'00'1110);
            let col1 = _mm_insert_ps(tmp, s, 0b01'01'1101);
            let col2 = _mm_insert_ps(tmp, s, 0b10'10'1011);
            let col3 = _mm_insert_ps(tmp, s, 0b11'11'0111);
            return {col0, col1, col2, col3};
        }

        [[nodiscard]] float scaleX() const noexcept {
            return s.x();
        }

        [[nodiscard]] friend mat::S operator*(mat::S const &lhs, mat::S const &rhs) noexcept {
            return mat::S{lhs.s * rhs.s};
        }

        [[nodiscard]] friend vec operator*(mat::S const &lhs, vec const &rhs) noexcept {
            return vec{lhs.s * rhs};
        }
    };

    /** Optimized translate matrix.
    */
    struct T {
        vec t;

        explicit T(vec rhs) noexcept :
            t(rhs) { ttauri_assume(rhs.is_vector()); }

        T(float x, float y, float z=0.0f) noexcept :
            t(x, y, z) {}

        operator mat () const noexcept {
            ttauri_assume(t.is_vector());
            let col0 = _mm_set_ss(1.0f);
            let col1 = _mm_permute_ps(col0, _MM_SHUFFLE(1,1,0,1));
            let col2 = _mm_permute_ps(col0, _MM_SHUFFLE(1,0,1,1));
            let col3 = _mm_insert_ps(t, col0, 0b00'11'0000);
            return {col0, col1, col2, col3};
        }

        [[nodiscard]] float scaleX() const noexcept {
            return 1.0f;
        }

        [[nodiscard]] friend mat::T operator*(mat::T const &lhs, mat::T const &rhs) noexcept {
            return mat::T{lhs.t + rhs.t};
        }

        [[nodiscard]] friend mat operator*(mat::T const &lhs, mat::S const &rhs) noexcept {
            let tmp = _mm_setzero_ps();
            let col0 = _mm_insert_ps(tmp, rhs.s, 0b00'00'1110);
            let col1 = _mm_insert_ps(tmp, rhs.s, 0b01'01'1101);
            let col2 = _mm_insert_ps(tmp, rhs.s, 0b10'10'1011);
            let col3 = _mm_insert_ps(lhs.t, rhs.s, 0b11'11'0000);
            return {col0, col1, col2, col3};
        }

        [[nodiscard]] friend mat operator*(mat::S const &lhs, mat::T const &rhs) noexcept {
            let tmp = _mm_setzero_ps();
            let col0 = _mm_insert_ps(tmp, lhs.s, 0b00'00'1110);
            let col1 = _mm_insert_ps(tmp, lhs.s, 0b01'01'1101);
            let col2 = _mm_insert_ps(tmp, lhs.s, 0b10'10'1011);
            let col3 = _mm_insert_ps(lhs.s * rhs.t, lhs.s, 0b11'11'0000);
            return {col0, col1, col2, col3};
        }

        [[nodiscard]] friend vec operator*(mat::T const &lhs, vec const &rhs) noexcept {
            return vec{lhs.t + rhs};
        }
    };

    constexpr size_t size() noexcept { return 4; }

    template<size_t I>
    [[nodiscard]] force_inline vec &get() noexcept {
        static_assert(I <= 3);
        if constexpr (I == 0) {
            return col0;
        } else if constexpr (I == 1) {
            return col1;
        } else if constexpr (I == 2) {
            return col2;
        } else {
            return col3;
        }
    }

    template<size_t I>
    [[nodiscard]] force_inline vec get() const noexcept {
        static_assert(I <= 3);
        if constexpr (I == 0) {
            return col0;
        } else if constexpr (I == 1) {
            return col1;
        } else if constexpr (I == 2) {
            return col2;
        } else {
            return col3;
        }
    }

    /** Transpose this matrix.
     */
    mat &transpose() noexcept {
        _MM_TRANSPOSE4_PS(col0, col1, col2, col3);
        return *this;
    }

    /** Extract scale.
     * This scale will only work with positive scale matrices.
     */
    [[nodiscard]] force_inline vec scale() const noexcept {
        auto tmp0_sq = _mm_mul_ps(col0, col0);
        auto tmp1_sq = _mm_mul_ps(col1, col1);
        auto tmp2_sq = _mm_mul_ps(col2, col2);
        auto tmp3_sq = _mm_setzero_ps();
        _MM_TRANSPOSE4_PS(tmp0_sq, tmp1_sq, tmp2_sq, tmp3_sq);
        let sum01_sq = _mm_add_ps(tmp0_sq, tmp1_sq);
        let sum012_sq = _mm_add_ps(sum01_sq, tmp2_sq);
        return _mm_sqrt_ps(sum012_sq);
    }

    /** Extract scale on x-axis.
    * This scale will only work with positive scale matrices 
    */
    [[nodiscard]] force_inline float scaleX() const noexcept {
        return length(col0);
    }

    /** Matrix/Vector multiplication.
     * Used for transforming vectors.
     */
    [[nodiscard]] force_inline friend vec operator*(mat const &lhs, vec const &rhs) noexcept {
        return
            (lhs.col0 * rhs.xxxx() + lhs.col1 * rhs.yyyy()) +
            (lhs.col2 * rhs.zzzz() + lhs.col3 * rhs.wwww());
    }

    /** Matrix/Vector multiplication.
    * Used for transforming vectors.
    */
    [[nodiscard]] force_inline friend rect operator*(mat const &lhs, rect const &rhs) noexcept {
        return rect::p1p2(lhs * rhs.p1(), lhs * rhs.p2());
    }

    /** Matrix/Matrix multiplication.
     */
    [[nodiscard]] friend mat operator*(mat const &lhs, mat const &rhs) noexcept {
        return {lhs * rhs.col0, lhs * rhs.col1, lhs * rhs.col2, lhs * rhs.col3};
    }

    /** Scale/Matrix multiplication.
    */
    [[nodiscard]] friend mat operator*(S const &lhs, mat const &rhs) noexcept {
        return {lhs.s * rhs.col0, lhs.s * rhs.col1, lhs.s * rhs.col2, lhs.s * rhs.col3};
    }

    /** Translate/Matrix multiplication.
    */
    [[nodiscard]] friend mat operator*(T const &lhs, mat const &rhs) noexcept {
        return {rhs.col0, rhs.col1, rhs.col2, lhs.t + rhs.col3};
    }

    [[nodiscard]] friend bool operator==(mat const &lhs, mat const &rhs) noexcept {
        return lhs.col0 == rhs.col0 && lhs.col1 == rhs.col1 && lhs.col2 == rhs.col2 && lhs.col3 == rhs.col3;
    }

    [[nodiscard]] friend bool operator!=(mat const &lhs, mat const &rhs) noexcept {
        return !(lhs == rhs);
    }

    /** Matrix transpose.
     */
    [[nodiscard]] friend mat transpose(mat rhs) noexcept {
        return rhs.transpose();
    }

    /** Create an identity matrix.
    */
    [[nodiscard]] static mat I() noexcept {
        let col0 = _mm_set_ss(1.0);
        let col1 = _mm_permute_ps(col0, _MM_SHUFFLE(3,3,0,3));
        let col2 = _mm_permute_ps(col0, _MM_SHUFFLE(3,0,3,3));
        let col3 = _mm_permute_ps(col0, _MM_SHUFFLE(0,3,3,3));
        return {col0, col1, col2, col3};
    }

    

    /** Create a 2D shearing matrix.
    */
    //[[nodiscard]] static mat S(float _00, float _01, float _10, float _11) noexcept {

    /** Create a rotation matrix.
     * @param N 0 = rotate around x-axis, 1=rotate around y-axis, 2=rotate around z-axis
     * @param rhs Angle in radials counter-clockwise.
     */
    template<int N=2, typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    [[nodiscard]] static mat R(T rhs) noexcept {
        let s = sin(numeric_cast<float>(rhs));
        let c = cos(numeric_cast<float>(rhs));
        let tmp1 = _mm_set_ps(c, s, 1.0f, 0.0f);
        let tmp2 = _mm_insert_ps(tmp1, _mm_set_ss(-s), 0b00'10'0000);

        if constexpr (N == 0) {
            let col0 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,0,0,1));
            let col1 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,2,3,0));
            let col2 = _mm_permute_ps(tmp2, _MM_SHUFFLE(0,3,2,0)); // -sin
            let col3 = _mm_permute_ps(tmp1, _MM_SHUFFLE(1,0,0,0));
            return {col0, col1, col2, col3};
        } else if constexpr (N == 1) {
            let col0 = _mm_permute_ps(tmp2, _MM_SHUFFLE(0,2,0,3)); // -sin
            let col1 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,0,1,0));
            let col2 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,3,0,2));
            let col3 = _mm_permute_ps(tmp1, _MM_SHUFFLE(1,0,0,0));
            return {col0, col1, col2, col3};
        } else {
            let col0 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,0,2,3));
            let col1 = _mm_permute_ps(tmp2, _MM_SHUFFLE(0,0,3,2)); // -sin
            let col2 = _mm_permute_ps(tmp1, _MM_SHUFFLE(0,1,0,0));
            let col3 = _mm_permute_ps(tmp1, _MM_SHUFFLE(1,0,0,0));
            return {col0, col1, col2, col3};
        }
    }

    [[nodiscard]] friend std::string to_string(mat const &rhs) noexcept {
        return fmt::format("[{}, {}, {}, {}]", rhs.col0, rhs.col1, rhs.col2, rhs.col3);
    }

    friend std::ostream &operator<<(std::ostream &lhs, mat const &rhs) noexcept {
        return lhs << to_string(rhs);
    }
};

template<typename M> struct is_mat : std::false_type {};
template<> struct is_mat<mat> : std::true_type {};
template<> struct is_mat<mat::T> : std::true_type {};
template<> struct is_mat<mat::S> : std::true_type {};

template<typename M>
inline constexpr bool is_mat_v = is_mat<M>::value;

}