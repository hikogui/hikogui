// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/rect.hpp"

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
    force_inline mat() noexcept {
        col0 = _mm_set_ss(1.0);
        col1 = _mm_permute_ps(col0, _MM_SHUFFLE(3,3,0,3));
        col2 = _mm_permute_ps(col0, _MM_SHUFFLE(3,0,3,3));
        col3 = _mm_permute_ps(col0, _MM_SHUFFLE(0,3,3,3));
    }

    force_inline mat(mat const &rhs) noexcept = default;
    force_inline mat &operator=(mat const &rhs) noexcept = default;
    force_inline mat(mat &&rhs) noexcept = default;
    force_inline mat &operator=(mat &&rhs) noexcept = default;

    /** Create a matrix for 4 vector-columns
     */
    force_inline mat(vec col0, vec col1, vec col2, vec col3) noexcept :
        col0(col0), col1(col1), col2(col2), col3(col3) {}

    constexpr size_t size() noexcept { return 4; }

    /** Transpose this matrix.
     */
    mat &transpose() noexcept {
        _MM_TRANSPOSE4_PS(col0, col1, col2, col3);
        return *this;
    }

    /** Matrix/Vector multiplication.
     * Used for transforming vectors.
     */
    [[nodiscard]] force_inline friend vec<N> operator*(mat const &lhs, vec<N> const &rhs) noexcept {
        return
            (lhs.col0 * rhs.xxxx() + lhs.col1 * rhs.yyyy()) +
            (lhs.col2 * rhs.zzzz() + lhs.col3 * rhs.wwww());
    }

    /** Matrix/Matrix multiplication.
     */
    [[nodiscard]] friend mat operator*(mat const &lhs, mat const &rhs) noexcept {
        return {lhs * rhs.col0, lhs * rhs.col1, lhs * rhs.col2, lhs * rhs.col3};
    }

    /** Matrix transpose.
     */
    [[nodiscard]] friend mat transpose(mat const rhs) noexcept {
        return rhs.transpose();
    }

    /** Create a translation matrix.
     */
    [[nodiscard]] static mat translate(vec rhs) noexcept {
        let col0 = _mm_set_ss(1.0f);
        let col1 = _mm_permute_ps(col0, _MM_SHUFFLE(1,1,0,1));
        let col2 = _mm_permute_ps(col0, _MM_SHUFFLE(1,0,1,1));
        let col3 = _mm_insert_ps(rhs, col0, 0b00'11'0000);
        return {col0, col1, col2, col3};
    }

    /** Create a scaling matrix.
     */
    [[nodiscard]] static mat scale(vec rhs) noexcept {
        let tmp = _mm_set_ps1(1.0f);
        let col0 = _mm_insert_ps(tmp, rhs, 0b00'00'1110);
        let col1 = _mm_insert_ps(tmp, rhs, 0b01'01'1101);
        let col2 = _mm_insert_ps(tmp, rhs, 0b10'10'1011);
        let col3 = _mm_insert_ps(tmp, tmp, 0b11'11'0111);
        return {col0, col1, col2, col3};
    }

    /** Create a scaling matrix.
     */
    [[nodiscard]] static mat scale(float rhs) noexcept {
        let _0001 = _mm_set_ss(1.0f);
        let _000s = _mm_set_ss(rhs);
        let _00s1 = _mm_insert_ps(_0001, _000s, 0b00'01'1100);

        let col0 = _mm_permute_ps(_00s1, _MM_SHUFFLE(2,2,2,1));
        let col1 = _mm_permute_ps(_00s1, _MM_SHUFFLE(2,2,1,2));
        let col2 = _mm_permute_ps(_00s1, _MM_SHUFFLE(2,1,2,2));
        let col3 = _mm_permute_ps(_00s1, _MM_SHUFFLE(0,2,2,2));
        return {col0, col1, col2, col3};
    }

    /** Create a rotation matrix.
     * @param N 0 = rotate around x-axis, 1=rotate around y-axis, 2=rotate around z-axis
     * @param rhs Angle in radials counter-clockwise.
     */
    template<int N=2>
    [[nodiscard]] static mat rotate(float rhs) noexcept {
        let s = sin(rhs);
        let c = cos(rhs);
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

};

}