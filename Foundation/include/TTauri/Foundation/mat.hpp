// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/aarect.hpp"
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
    force_inline mat(vec col0, vec col1, vec col2, vec col3=vec{0.0f, 0.0f, 0.0f, 1.0f}) noexcept :
        col0(col0), col1(col1), col2(col2), col3(col3) {}

    /** Construct a matrix from the individual values.
     * The arguments are ordered so that 4 rows of 4 values will
     * construct the matrix visually in the same way as common
     * mathamatics papers.
     */
    force_inline mat(
        float i00, float i10, float i20, float i30,
        float i01, float i11, float i21, float i31,
        float i02, float i12, float i22, float i32,
        float i03, float i13, float i23, float i33
    ) :
        col0(i00, i01, i02, i03),
        col1(i10, i11, i12, i13),
        col2(i20, i21, i22, i23),
        col3(i30, i31, i32, i33) {}

    /** Optimized scale matrix.
    */
    struct S {
        vec s;

        explicit S(vec rhs) noexcept :
            s(rhs) { ttauri_assume(rhs.is_point()); }

        S(float x, float y, float z=1.0f) noexcept :
            s(x, y, z, 1.0f) {}

        /** Create a scaling matrix.
        */
        operator mat () const noexcept {
            ttauri_assume(s.is_point());
            let tmp = _mm_setzero_ps();
            let c0 = _mm_insert_ps(tmp, s, 0b00'00'1110);
            let c1 = _mm_insert_ps(tmp, s, 0b01'01'1101);
            let c2 = _mm_insert_ps(tmp, s, 0b10'10'1011);
            let c3 = _mm_insert_ps(tmp, s, 0b11'11'0111);
            return {c0, c1, c2, c3};
        }

        [[nodiscard]] friend S operator*(S const &lhs, S const &rhs) noexcept {
            return S{lhs.s * rhs.s};
        }

        [[nodiscard]] friend vec operator*(S const &lhs, vec const &rhs) noexcept {
            return vec{lhs.s * rhs};
        }

        /** Matrix/Vector multiplication.
        * Used for transforming vectors.
        */
        [[nodiscard]] force_inline friend aarect operator*(S const &lhs, aarect const &rhs) noexcept {
            return aarect::p1p2(lhs.s * rhs.p1(), lhs.s * rhs.p2());
        }

        [[nodiscard]] friend rect operator*(S const &lhs, rect const &rhs) noexcept {
            return rect{
                lhs.s * rhs.corner<0>(),
                lhs.s * rhs.corner<1>(),
                lhs.s * rhs.corner<2>(),
                lhs.s * rhs.corner<3>()
            };
        }

        /** Invert matrix.
        */
        [[nodiscard]] friend S operator~(S const &rhs) noexcept {
            return S{vec{_mm_rcp_ps(rhs.s)}};
        }
    };

    /** Optimized translate matrix.
    */
    struct T {
        vec t;

        T() noexcept : t() {}
        T(T const &rhs) noexcept = default;
        T(T &&rhs) noexcept = default;
        T &operator=(T const &rhs) noexcept = default;
        T &operator=(T &&rhs) noexcept = default;

        explicit T(vec rhs) noexcept :
            t(rhs) { ttauri_assume(rhs.is_vector()); }

        T(float x, float y, float z=0.0f) noexcept :
            t(x, y, z) {}

        operator mat () const noexcept {
            ttauri_assume(t.is_vector());
            let c0 = _mm_set_ss(1.0f);
            let c1 = _mm_permute_ps(c0, _MM_SHUFFLE(1,1,0,1));
            let c2 = _mm_permute_ps(c0, _MM_SHUFFLE(1,0,1,1));
            let c3 = _mm_insert_ps(t, c0, 0b00'11'0000);
            return {c0, c1, c2, c3};
        }

        [[nodiscard]] friend T operator*(T const &lhs, T const &rhs) noexcept {
            return T{lhs.t + rhs.t};
        }

        [[nodiscard]] friend mat operator*(T const &lhs, mat::S const &rhs) noexcept {
            let tmp = _mm_setzero_ps();
            let col0 = _mm_insert_ps(tmp, rhs.s, 0b00'00'1110);
            let col1 = _mm_insert_ps(tmp, rhs.s, 0b01'01'1101);
            let col2 = _mm_insert_ps(tmp, rhs.s, 0b10'10'1011);
            let col3 = _mm_insert_ps(lhs.t, rhs.s, 0b11'11'0000);
            return {col0, col1, col2, col3};
        }

        [[nodiscard]] friend mat operator*(mat::S const &lhs, T const &rhs) noexcept {
            let tmp = _mm_setzero_ps();
            let col0 = _mm_insert_ps(tmp, lhs.s, 0b00'00'1110);
            let col1 = _mm_insert_ps(tmp, lhs.s, 0b01'01'1101);
            let col2 = _mm_insert_ps(tmp, lhs.s, 0b10'10'1011);
            let col3 = _mm_insert_ps(lhs.s * rhs.t, lhs.s, 0b11'11'0000);
            return {col0, col1, col2, col3};
        }

        [[nodiscard]] friend vec operator*(T const &lhs, vec const &rhs) noexcept {
            return vec{lhs.t + rhs};
        }

        /** Matrix/aarect multiplication.
        */
        [[nodiscard]] force_inline friend rect operator*(T const &lhs, aarect const &rhs) noexcept {
            return rect{
                lhs.t + rhs.corner<0>(),
                lhs.t + rhs.corner<1>(),
                lhs.t + rhs.corner<2>(),
                lhs.t + rhs.corner<3>()
            };
        }

        [[nodiscard]] friend rect operator*(T const &lhs, rect const &rhs) noexcept {
            return rect{
                lhs.t + rhs.corner<0>(),
                lhs.t + rhs.corner<1>(),
                lhs.t + rhs.corner<2>(),
                lhs.t + rhs.corner<3>()
            };
        }

        /** Invert matrix.
        */
        [[nodiscard]] friend T operator~(T const &rhs) noexcept {
            return T{-rhs.t};
        }
    };

    /** Optimized 2D translate matrix.
    */
    struct T2 {
        vec t;

        T2() noexcept : t() {}

        explicit T2(vec rhs) noexcept : t(rhs) {
            ttauri_assume(rhs.is_vector());
            ttauri_assume(rhs.z() == 0.0);
        }

        T2(float x, float y) noexcept :
            t(x, y, 0.0) {}

        operator mat::T () const noexcept {
            return mat::T{t};
        }

        operator mat () const noexcept {
            ttauri_assume(t.is_vector());
            let c0 = _mm_set_ss(1.0f);
            let c1 = _mm_permute_ps(c0, _MM_SHUFFLE(1,1,0,1));
            let c2 = _mm_permute_ps(c0, _MM_SHUFFLE(1,0,1,1));
            let c3 = _mm_insert_ps(t, c0, 0b00'11'0000);
            return {c0, c1, c2, c3};
        }

        [[nodiscard]] friend T2 operator*(T2 const &lhs, T2 const &rhs) noexcept {
            return T2{lhs.t + rhs.t};
        }

        [[nodiscard]] friend mat::T operator*(mat::T const &lhs, T2 const &rhs) noexcept {
            return mat::T{lhs.t + rhs.t};
        }

        [[nodiscard]] friend mat::T operator*(T2 const &lhs, mat::T const &rhs) noexcept {
            return mat::T{lhs.t + rhs.t};
        }

        [[nodiscard]] friend mat operator*(T2 const &lhs, mat::S const &rhs) noexcept {
            let tmp = _mm_setzero_ps();
            let col0 = _mm_insert_ps(tmp, rhs.s, 0b00'00'1110);
            let col1 = _mm_insert_ps(tmp, rhs.s, 0b01'01'1101);
            let col2 = _mm_insert_ps(tmp, rhs.s, 0b10'10'1011);
            let col3 = _mm_insert_ps(lhs.t, rhs.s, 0b11'11'0000);
            return {col0, col1, col2, col3};
        }

        [[nodiscard]] friend mat operator*(mat::S const &lhs, T2 const &rhs) noexcept {
            let tmp = _mm_setzero_ps();
            let col0 = _mm_insert_ps(tmp, lhs.s, 0b00'00'1110);
            let col1 = _mm_insert_ps(tmp, lhs.s, 0b01'01'1101);
            let col2 = _mm_insert_ps(tmp, lhs.s, 0b10'10'1011);
            let col3 = _mm_insert_ps(lhs.s * rhs.t, lhs.s, 0b11'11'0000);
            return {col0, col1, col2, col3};
        }

        [[nodiscard]] friend vec operator*(T2 const &lhs, vec const &rhs) noexcept {
            return vec{lhs.t + rhs};
        }

        [[nodiscard]] friend aarect operator*(T2 const &lhs, aarect const &rhs) noexcept {
            return aarect::p1p2(lhs.t + rhs.p1(), lhs.t + rhs.p2());
        }

        [[nodiscard]] friend rect operator*(T2 const &lhs, rect const &rhs) noexcept {
            return rect{
                lhs.t + rhs.corner<0>(),
                lhs.t + rhs.corner<1>(),
                lhs.t + rhs.corner<2>(),
                lhs.t + rhs.corner<3>()
            };
        }

        /** Invert matrix.
        */
        [[nodiscard]] friend T2 operator~(T2 const &rhs) noexcept {
            return T2{-rhs.t};
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

    /** Rotation around z axis is a multiple of 90 degree.
    */
    [[nodiscard]] bool is_z_rot90() const noexcept {
        let xyxy = _mm_shuffle_ps(col0, col1, _MM_SHUFFLE(1,0,1,0));
        let result = _mm_cmpeq_ps(xyxy, _mm_setzero_ps());
        let result_ = _mm_movemask_ps(result);
        return (result_ == 0b1001) || (result_ == 0b0110);
    }

    /** Transpose this matrix.
     */
    mat &transpose_() noexcept {
        _MM_TRANSPOSE4_PS(col0, col1, col2, col3);
        return *this;
    }

    /** Matrix/Vector multiplication.
     * Used for transforming vectors.
     */
    [[nodiscard]] force_inline friend vec operator*(mat const &lhs, vec const &rhs) noexcept {
        return
            (lhs.col0 * rhs.xxxx() + lhs.col1 * rhs.yyyy()) +
            (lhs.col2 * rhs.zzzz() + lhs.col3 * rhs.wwww());
    }

    /** Matrix/aarect multiplication.
    */
    [[nodiscard]] force_inline friend rect operator*(mat const &lhs, aarect const &rhs) noexcept {
        return rect{
            lhs * rhs.corner<0>(),
            lhs * rhs.corner<1>(),
            lhs * rhs.corner<2>(),
            lhs * rhs.corner<3>()
        };
    }

    /** Matrix/rect multiplication.
    */
    [[nodiscard]] force_inline friend rect operator*(mat const &lhs, rect const &rhs) noexcept {
        return rect{
            lhs * rhs.corner<0>(),
            lhs * rhs.corner<1>(),
            lhs * rhs.corner<2>(),
            lhs * rhs.corner<3>()
        };
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

    /** Matrix transpose.
    */
    [[nodiscard]] friend mat transpose(mat rhs) noexcept {
        return rhs.transpose_();
    }

    /** Invert matrix.
     */
    [[nodiscard]] friend mat operator~(mat const &rhs) {
        //                   rc
        //var s0 : Number = i00 * i11 - 
        //                  i10 * i01;
        //var c0 : Number = i20 * i31 -
        //                  i30 * i21;
        let s0c0 = rhs.col0 * rhs.col1.yxwz();

        //var s1 : Number = i00 * i12 -
        //                  i10 * i02;
        //var c1 : Number = i20 * i32 -
        //                  i30 * i22;
        let s1c1 = rhs.col0 * rhs.col2.yxwz();
        let s0c0s1c1 = hsub(s0c0, s1c1);

        //var s2 : Number = i00 * i13 -
        //                  i10 * i03;
        //var c2 : Number = i20 * i33 -
        //                  i30 * i23;
        let s2c2 = rhs.col0 * rhs.col3.yxwz();

        //var s3 : Number = i01 * i12 -
        //                  i11 * i02;
        //var c3 : Number = i21 * i32 -
        //                  i31 * i22;
        let s3c3 = rhs.col1 * rhs.col2.yxwz();
        let s2c2s3c3 = hsub(s2c2, s3c3);

        //var s4 : Number = i01 * i13 -
        //                  i11 * i03;
        //var c4 : Number = i21 * i33 -
        //                  i31 * i23;
        let s4c4 = rhs.col1 * rhs.col3.yxwz();

        //var s5 : Number = i02 * i13 -
        //                  i12 * i03;
        //var c5 : Number = i22 * i33 -
        //                  i32 * i23;
        let s5c5 = rhs.col2 * rhs.col3.yxwz();
        let s4c4s5c5 = hsub(s4c4, s5c5);

        // det = (s0 * c5 +
        //       -s1 * c4 +
        //        s2 * c3 +
        //        s3 * c2 +
        //       -s4 * c1 +
        //        s5 * c0)
        let s0123 = s0c0s1c1.xz00() + s2c2s3c3._00xz();
        let s45__ = s4c4s5c5.xz00();

        let c5432 = s4c4s5c5.wy00() + s2c2s3c3._00wy();
        let c10__ = s0c0s1c1.wy00();

        let det_prod_half0 = neg<0,1,0,0>(s0123 * c5432);
        let det_prod_half1 = neg<1,0,0,0>(s45__ * c10__);

        let det_sum0 = hadd(det_prod_half0, det_prod_half1);
        let det_sum1 = hadd(det_sum0, det_sum0);
        let det = hadd(det_sum1, det_sum1).xxxx();

        if (det.x() == 0.0f) {
            TTAURI_THROW(math_error("Divide by zero"));
        }

        let invdet = reciprocal(det);

        let t = transpose(rhs);

        //   rc     rc          rc          rc
        //m.i00 = (i11 *  c5 + i12 * -c4 + i13 *  c3) * invdet;
        //m.i10 = (i10 * -c5 + i12 *  c2 + i13 * -c1) * invdet;
        //m.i20 = (i10 *  c4 + i11 * -c2 + i13 *  c0) * invdet;
        //m.i30 = (i10 * -c3 + i11 *  c1 + i12 * -c0) * invdet;
        auto tmp_c5543 = neg<0,1,0,1>(c5432.xxyz());
        auto tmp_c4221 = neg<1,0,1,0>(c5432.yww0() + c10__._000x());
        auto tmp_c3100 = neg<0,1,0,1>(c5432.z000() + c10__._0xyy());
        let inv_col0 = (
            (t.col1.yxxx() * tmp_c5543) +
            (t.col1.zzyy() * tmp_c4221) +
            (t.col1.wwwz() * tmp_c3100)
        ) * invdet;

        //m.i01 = (i01 * -c5 + i02 *  c4 + i03 * -c3) * invdet;
        //m.i11 = (i00 *  c5 + i02 * -c2 + i03 *  c1) * invdet;
        //m.i21 = (i00 * -c4 + i01 *  c2 + i03 * -c0) * invdet;
        //m.i31 = (i00 *  c3 + i01 * -c1 + i02 *  c0) * invdet;
        tmp_c5543 = -tmp_c5543;
        tmp_c4221 = -tmp_c4221;
        tmp_c3100 = -tmp_c3100;
        let inv_col1 = (
            (t.col0.yxxx() * tmp_c5543) +
            (t.col0.zzyy() * tmp_c4221) +
            (t.col0.wwwz() * tmp_c3100)
        ) * invdet;

        //m.i02 = (i31 *  s5 + i32 * -s4 + i33 *  s3) * invdet;
        //m.i12 = (i30 * -s5 + i32 *  s2 + i33 * -s1) * invdet;
        //m.i22 = (i30 *  s4 + i31 * -s2 + i33 *  s0) * invdet;
        //m.i32 = (i30 * -s3 + i31 *  s1 + i32 * -s0) * invdet;
        auto tmp_s5543 = neg<0,1,0,1>(s45__.yyx0() + s0123._000w());
        auto tmp_s4221 = neg<1,0,1,0>(s45__.x000() + s0123._0zzy());
        auto tmp_s3100 = neg<0,1,0,1>(s0123.wyxx());
        let inv_col2 = (
            (t.col3.yxxx() * tmp_s5543) +
            (t.col3.zzyy() * tmp_s4221) +
            (t.col3.wwwz() * tmp_s3100)
        ) * invdet;

        //m.i03 = (i21 * -s5 + i22 *  s4 + i23 * -s3) * invdet;
        //m.i13 = (i20 *  s5 + i22 * -s2 + i23 *  s1) * invdet;
        //m.i23 = (i20 * -s4 + i21 *  s2 + i23 * -s0) * invdet;
        //m.i33 = (i20 *  s3 + i21 * -s1 + i22 *  s0) * invdet;
        tmp_s5543 = -tmp_s5543;
        tmp_s4221 = -tmp_s4221;
        tmp_s3100 = -tmp_s3100;
        let inv_col3 = (
            (t.col2.yxxx() * tmp_s5543) +
            (t.col2.zzyy() * tmp_s4221) +
            (t.col2.wwwz() * tmp_s3100)
        ) * invdet;

        return {inv_col0, inv_col1, inv_col2, inv_col3};
    }

    [[nodiscard]] friend bool operator==(mat const &lhs, mat const &rhs) noexcept {
        return lhs.col0 == rhs.col0 && lhs.col1 == rhs.col1 && lhs.col2 == rhs.col2 && lhs.col3 == rhs.col3;
    }

    [[nodiscard]] friend bool operator!=(mat const &lhs, mat const &rhs) noexcept {
        return !(lhs == rhs);
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

    [[nodiscard]] static mat RGBtoXYZ(
        float wx, float wy,
        float rx, float ry,
        float gx, float gy,
        float bx, float by) noexcept
    {
        let w = vec{wx, wy, 1.0f - wx - wy};
        let r = vec{rx, ry, 1.0f - rx - ry};
        let g = vec{gx, gy, 1.0f - gx - gy};
        let b = vec{bx, by, 1.0f - bx - by};

        // Calculate whitepoint's tristimulus values from coordinates
        let W = vec{
            1.0f * (w.x() / w.y()),
            1.0f,
            1.0f * (w.z() / w.y())
        };

        // C is the chromaticity matrix.
        let C = mat{r, g, b};

        // solve tristimulus sums.
        let S = mat::S{vec::point(~C * W)};

        return C * S;
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

    /** Align a rectangle within another rectangle.
    * @param outside The outside rectangle
    * @param inside The inside rectangle; to be aligned.
    * @param alignment How the inside rectangle should be aligned.
    * @return Translation matrix to draw the inside rectangle as if the inside
    *         rectangle's left-bottom corner is positioned at the origin.
    */
    [[nodiscard]] static mat::T align(aarect outside, aarect inside, Alignment alignment) noexcept {
        return mat::T{aarect::_align(outside, inside, alignment).offset()};
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
template<> struct is_mat<mat::T2> : std::true_type {};
template<> struct is_mat<mat::S> : std::true_type {};

template<typename M>
inline constexpr bool is_mat_v = is_mat<M>::value;

}
