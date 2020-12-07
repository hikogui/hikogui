// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "numeric_array.hpp"
#include "aarect.hpp"
#include "rect.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <ostream>

namespace tt {

/** A 4x4 matrix.
 * You can use this to transform f32x4 (which has 4 elements)
 */
class mat {
    f32x4 col0;
    f32x4 col1;
    f32x4 col2;
    f32x4 col3;

public:
    /** Create an identity matrix.
     */
    mat() noexcept {}
    mat(mat const &rhs) noexcept = default;
    mat &operator=(mat const &rhs) noexcept = default;
    mat(mat &&rhs) noexcept = default;
    mat &operator=(mat &&rhs) noexcept = default;

    /** Create a matrix for 4 vector-columns
     */
    mat(f32x4 col0, f32x4 col1, f32x4 col2, f32x4 col3=f32x4{0.0f, 0.0f, 0.0f, 1.0f}) noexcept :
        col0(col0), col1(col1), col2(col2), col3(col3) {}

    /** Construct a matrix from the individual values.
     * The arguments are ordered so that 4 rows of 4 values will
     * construct the matrix visually in the same way as common
     * mathamatics papers.
     */
    mat(
        float i00, float i10, float i20, float i30,
        float i01, float i11, float i21, float i31,
        float i02, float i12, float i22, float i32,
        float i03, float i13, float i23, float i33
    ) :
        col0({i00, i01, i02, i03}),
        col1({i10, i11, i12, i13}),
        col2({i20, i21, i22, i23}),
        col3({i30, i31, i32, i33}) {}

    /** Optimized scale matrix.
    */
    struct S {
        f32x4 s;

        explicit S(f32x4 rhs) noexcept :
            s(rhs)
        {
            tt_assume(rhs.is_point());
        }

        S(float x, float y, float z=1.0f) noexcept :
            s({x, y, z, 1}) {}

        /** Get a scaling matrix to uniformly scale a needle to fit in the haystack.
         */
        static S uniform2D(f32x4 haystack, f32x4 needle) noexcept {
            tt_assume(haystack.x() != 0.0f && haystack.y() != 0.0f);
            tt_assume(needle.x() != 0.0f && needle.y() != 0.0f);

            ttlet non_uniform_scale = haystack.xyxy() / needle.xyxy();
            ttlet uniform_scale = std::min(non_uniform_scale.x(), non_uniform_scale.y());
            return S{f32x4{uniform_scale, uniform_scale, 1.0f, 1.0f}};
        }

        /** Create a scaling matrix.
        */
        operator mat () const noexcept {
            tt_assume(s.is_point());
            return { s.x000(), s._0y00(), s._00z0(), s._000w() };
        }

        [[nodiscard]] friend S operator*(S const &lhs, S const &rhs) noexcept {
            return S{lhs.s * rhs.s};
        }

        [[nodiscard]] friend f32x4 operator*(S const &lhs, f32x4 const &rhs) noexcept {
            return lhs.s * rhs;
        }

        /** Matrix/Vector multiplication.
        * Used for transforming vectors.
        */
        [[nodiscard]] friend aarect operator*(S const &lhs, aarect const &rhs) noexcept {
            return aarect::p0p3(lhs.s * rhs.p0(), lhs.s * rhs.p3());
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
            return S{rcp(rhs.s)};
        }
    };

    /** Optimized translate matrix.
    */
    struct T {
        f32x4 t;

        T() noexcept : t() {}
        T(T const &rhs) noexcept = default;
        T(T &&rhs) noexcept = default;
        T &operator=(T const &rhs) noexcept = default;
        T &operator=(T &&rhs) noexcept = default;

        explicit T(f32x4 rhs) noexcept :
            t(rhs) { tt_assume(rhs.is_vector()); }

        T(float x, float y, float z=0.0f) noexcept :
            t({x, y, z}) {}

        operator mat () const noexcept {
            tt_assume(t.is_vector());
            return { t._1000(), t._0100(), t._0010(), t.xyz1() };
        }

        [[nodiscard]] friend T operator*(T const &lhs, T const &rhs) noexcept {
            return T{lhs.t + rhs.t};
        }

        [[nodiscard]] friend mat operator*(T const &lhs, mat::S const &rhs) noexcept {
            return { rhs.s.x000(), rhs.s._0y00(), rhs.s._00z0(), lhs.t.xyz1() };
        }

        [[nodiscard]] friend mat operator*(mat::S const &lhs, T const &rhs) noexcept {
            return { lhs.s.x000(), lhs.s._0y00(), lhs.s._00z0(), lhs.s * rhs.t.xyz1() };
        }

        [[nodiscard]] friend f32x4 operator*(T const &lhs, f32x4 const &rhs) noexcept {
            return lhs.t + rhs;
        }

        /** Matrix/aarect multiplication.
        */
        [[nodiscard]] friend rect operator*(T const &lhs, aarect const &rhs) noexcept {
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
        f32x4 t;

        T2() noexcept : t() {}

        explicit T2(f32x4 rhs) noexcept : t(rhs) {
            tt_assume(rhs.is_vector());
            tt_assume(rhs.z() == 0.0f);
        }

        explicit T2(aarect rhs) noexcept : t(rhs.offset()) {
            tt_assume(rhs.offset().is_vector());
            tt_assume(rhs.offset().z() == 0.0f);
        }

        T2(float x, float y) noexcept :
            t({x, y, 0.0f}) {}

        operator mat::T () const noexcept {
            return mat::T{t};
        }

        operator mat () const noexcept {
            tt_assume(t.is_vector());
            return { t._1000(), t._0100(), t._0010(), t.xyz1() };
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
            return { rhs.s.x000(), rhs.s._0y00(), rhs.s._00z0(), lhs.t.xyz1() };
        }

        [[nodiscard]] friend mat operator*(mat::S const &lhs, T2 const &rhs) noexcept {
            return { lhs.s.x000(), lhs.s._0y00(), lhs.s._00z0(), lhs.s * rhs.t.xyz1() };
        }

        [[nodiscard]] friend f32x4 operator*(T2 const &lhs, f32x4 const &rhs) noexcept {
            return lhs.t + rhs;
        }

        [[nodiscard]] friend aarect operator*(T2 const &lhs, aarect const &rhs) noexcept {
            return aarect::p0p3(lhs.t + rhs.p0(), lhs.t + rhs.p3());
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
    [[nodiscard]] f32x4 &get() noexcept {
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
    [[nodiscard]] f32x4 get() const noexcept {
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
        ttlet xyxy = col0.xy00() + col1._00xy();
        ttlet result = eq(xyxy, f32x4{});
        return (result == 0b1001) || (result == 0b0110);
    }

    /** Matrix/Vector multiplication.
     * Used for transforming vectors.
     */
    [[nodiscard]] friend f32x4 operator*(mat const &lhs, f32x4 const &rhs) noexcept {
        return
            (lhs.col0 * rhs.xxxx() + lhs.col1 * rhs.yyyy()) +
            (lhs.col2 * rhs.zzzz() + lhs.col3 * rhs.wwww());
    }

    /** Matrix/aarect multiplication.
    */
    [[nodiscard]] friend rect operator*(mat const &lhs, aarect const &rhs) noexcept {
        return rect{
            lhs * rhs.corner<0>(),
            lhs * rhs.corner<1>(),
            lhs * rhs.corner<2>(),
            lhs * rhs.corner<3>()
        };
    }

    /** Matrix/rect multiplication.
    */
    [[nodiscard]] friend rect operator*(mat const &lhs, rect const &rhs) noexcept {
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
        auto tmp = transpose(rhs.col0, rhs.col1, rhs.col2, rhs.col3);
        return {std::get<0>(tmp), std::get<1>(tmp), std::get<2>(tmp), std::get<3>(tmp)};
    }

    /** Invert matrix.
     */
    [[nodiscard]] friend mat operator~(mat const &rhs) {
        //                   rc
        //var s0 : Number = i00 * i11 - 
        //                  i10 * i01;
        //var c0 : Number = i20 * i31 -
        //                  i30 * i21;
        ttlet s0c0 = rhs.col0 * rhs.col1.yxwz();

        //var s1 : Number = i00 * i12 -
        //                  i10 * i02;
        //var c1 : Number = i20 * i32 -
        //                  i30 * i22;
        ttlet s1c1 = rhs.col0 * rhs.col2.yxwz();
        ttlet s0c0s1c1 = hsub(s0c0, s1c1);

        //var s2 : Number = i00 * i13 -
        //                  i10 * i03;
        //var c2 : Number = i20 * i33 -
        //                  i30 * i23;
        ttlet s2c2 = rhs.col0 * rhs.col3.yxwz();

        //var s3 : Number = i01 * i12 -
        //                  i11 * i02;
        //var c3 : Number = i21 * i32 -
        //                  i31 * i22;
        ttlet s3c3 = rhs.col1 * rhs.col2.yxwz();
        ttlet s2c2s3c3 = hsub(s2c2, s3c3);

        //var s4 : Number = i01 * i13 -
        //                  i11 * i03;
        //var c4 : Number = i21 * i33 -
        //                  i31 * i23;
        ttlet s4c4 = rhs.col1 * rhs.col3.yxwz();

        //var s5 : Number = i02 * i13 -
        //                  i12 * i03;
        //var c5 : Number = i22 * i33 -
        //                  i32 * i23;
        ttlet s5c5 = rhs.col2 * rhs.col3.yxwz();
        ttlet s4c4s5c5 = hsub(s4c4, s5c5);

        // det = (s0 * c5 +
        //       -s1 * c4 +
        //        s2 * c3 +
        //        s3 * c2 +
        //       -s4 * c1 +
        //        s5 * c0)
        ttlet s0123 = s0c0s1c1.xz00() + s2c2s3c3._00xz();
        ttlet s45__ = s4c4s5c5.xz00();

        ttlet c5432 = s4c4s5c5.wy00() + s2c2s3c3._00wy();
        ttlet c10__ = s0c0s1c1.wy00();

        ttlet det_prod_half0 = neg<0,1,0,0>(s0123 * c5432);
        ttlet det_prod_half1 = neg<1,0,0,0>(s45__ * c10__);

        ttlet det_sum0 = hadd(det_prod_half0, det_prod_half1);
        ttlet det_sum1 = hadd(det_sum0, det_sum0);
        ttlet det = hadd(det_sum1, det_sum1).xxxx();

        if (det.x() == 0.0f) {
            throw std::domain_error("Divide by zero");
        }

        ttlet invdet = rcp(det);

        ttlet t = transpose(rhs);

        //   rc     rc          rc          rc
        //m.i00 = (i11 *  c5 + i12 * -c4 + i13 *  c3) * invdet;
        //m.i10 = (i10 * -c5 + i12 *  c2 + i13 * -c1) * invdet;
        //m.i20 = (i10 *  c4 + i11 * -c2 + i13 *  c0) * invdet;
        //m.i30 = (i10 * -c3 + i11 *  c1 + i12 * -c0) * invdet;
        auto tmp_c5543 = neg<0,1,0,1>(c5432.xxyz());
        auto tmp_c4221 = neg<1,0,1,0>(c5432.yww0() + c10__._000x());
        auto tmp_c3100 = neg<0,1,0,1>(c5432.z000() + c10__._0xyy());
        ttlet inv_col0 = (
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
        ttlet inv_col1 = (
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
        ttlet inv_col2 = (
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
        ttlet inv_col3 = (
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
        f32x4 tmp;
        return { tmp._1000(), tmp._0100(), tmp._0010(), tmp._0001() };
    }

    [[nodiscard]] static mat RGBtoXYZ(
        float wx, float wy,
        float rx, float ry,
        float gx, float gy,
        float bx, float by) noexcept
    {
        ttlet w = f32x4{wx, wy, 1.0f - wx - wy};
        ttlet r = f32x4{rx, ry, 1.0f - rx - ry};
        ttlet g = f32x4{gx, gy, 1.0f - gx - gy};
        ttlet b = f32x4{bx, by, 1.0f - bx - by};

        // Calculate whitepoint's tristimulus values from coordinates
        ttlet W = f32x4{
            1.0f * (w.x() / w.y()),
            1.0f,
            1.0f * (w.z() / w.y())
        };

        // C is the chromaticity matrix.
        ttlet C = mat{r, g, b};

        // solve tristimulus sums.
        ttlet S = mat::S{f32x4::point(~C * W)};

        return C * S;
    }

    /** Create a 2D shearing matrix.
     *
     * @param _00 row 0, col 0
     * @param _01 row 0, col 1
     * @param _10 row 1, col 0
     * @param _11 row 1, col 1
    */
    [[nodiscard]] static mat shear(float _00, float _01, float _10, float _11) noexcept {
        ttlet c0 = f32x4{_00, _10};
        ttlet c1 = f32x4{_01, _11};
        return { c0, c1, c0._0010(), c0._0001() };
    }

    /** Create a rotation matrix.
     * @param N 0 = rotate around x-axis, 1=rotate around y-axis, 2=rotate around z-axis
     * @param rhs Angle in radials counter-clockwise.
     */
    template<int N=2, typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    [[nodiscard]] static mat R(T rhs) noexcept {
        ttlet s = sin(narrow_cast<float>(rhs));
        ttlet c = cos(narrow_cast<float>(rhs));
        ttlet tmp = f32x4{c, s, -s};

        if constexpr (N == 0) {
            return { tmp._1000(), tmp._0xy0(), tmp._0zx0(), tmp._0001() };
        } else if constexpr (N == 1) {
            return { tmp.x0z0(), tmp._0100(), tmp.x0y0(), tmp._0001() };
        } else {
            return { tmp.xy00(), tmp.zx00(), tmp._0010(), tmp._0001() };
        }
    }

    /** Align a rectangle within another rectangle.
    * @param haystack The outside rectangle
    * @param needle The inside rectangle; to be aligned.
    * @param alignment How the inside rectangle should be aligned.
    * @return Translation matrix to move and align the needle in the haystack
    */
    [[nodiscard]] static mat::T align(aarect haystack, aarect needle, alignment alignment) noexcept {
        return mat::T{
            aarect::_align(haystack, needle, alignment).offset() -
            needle.offset()
        };
    }

    [[nodiscard]] static mat uniform2D_scale_and_translate(aarect haystack, aarect needle, alignment alignment) noexcept {
        ttlet scale = S::uniform2D(haystack.extent(), needle.extent());
        ttlet scaled_needle = scale * needle;
        ttlet translation = align(haystack, scaled_needle, alignment);
        return translation * scale;
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
