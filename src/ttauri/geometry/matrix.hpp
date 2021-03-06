// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "vector.hpp"
#include "extent.hpp"
#include "point.hpp"
#include "rectangle.hpp"
#include "axis_aligned_rectangle.hpp"
#include "../color/color.hpp"

namespace tt {
namespace geo {

template<int D>
class matrix {
public:
    static_assert(D == 2 || D == 3, "Only 2D or 3D rotation-matrices are supported");

    constexpr matrix(matrix const &) noexcept = default;
    constexpr matrix(matrix &&) noexcept = default;
    constexpr matrix &operator=(matrix const &) noexcept = default;
    constexpr matrix &operator=(matrix &&) noexcept = default;

    constexpr matrix() noexcept :
        _col0(1.0f, 0.0f, 0.0f, 0.0f),
        _col1(0.0f, 1.0f, 0.0f, 0.0f),
        _col2(0.0f, 0.0f, 1.0f, 0.0f),
        _col3(0.0f, 0.0f, 0.0f, 1.0f){};

    constexpr matrix(f32x4 col0, f32x4 col1, f32x4 col2, f32x4 col3 = f32x4{0.0f, 0.0f, 0.0f, 1.0f}) noexcept :
        _col0(col0), _col1(col1), _col2(col2), _col3(col3)
    {
    }

    constexpr matrix(vector3 col0, vector3 col1, vector3 col2) noexcept requires(D == 3) :
        _col0(static_cast<f32x4>(col0)),
        _col1(static_cast<f32x4>(col1)),
        _col2(static_cast<f32x4>(col2)),
        _col3{0.0f, 0.0f, 0.0f, 1.0f}
    {
    }

    constexpr matrix(
        float c0r0,
        float c1r0,
        float c2r0,
        float c0r1,
        float c1r1,
        float c2r1,
        float c0r2,
        float c1r2,
        float c2r2) noexcept requires(D == 3) :
        _col0(c0r0, c0r1, c0r2, 0.0f), _col1(c1r0, c1r1, c1r2, 0.0f), _col2(c2r0, c2r1, c2r2, 0.0f), _col3(0.0f, 0.0f, 0.0f, 1.0f)
    {
    }

    constexpr matrix(
        float c0r0,
        float c1r0,
        float c2r0,
        float c3r0,
        float c0r1,
        float c1r1,
        float c2r1,
        float c3r1,
        float c0r2,
        float c1r2,
        float c2r2,
        float c3r2,
        float c0r3,
        float c1r3,
        float c2r3,
        float c3r3) noexcept requires(D == 3) :
        _col0(c0r0, c0r1, c0r2, c0r3), _col1(c1r0, c1r1, c1r2, c1r3), _col2(c2r0, c2r1, c2r2, c2r3), _col3(c3r0, c3r1, c3r2, c3r3)
    {
    }

    template<int E>
    requires(E < D) [[nodiscard]] constexpr matrix(matrix<E> const &other) noexcept :
        _col0(get<0>(other)), _col1(get<1>(other)), _col2(get<2>(other)), _col3(get<3>(other))
    {
    }

    template<int E>
    requires(E < D) constexpr matrix &operator=(matrix<E> const &rhs) noexcept
    {
        _col0 = get<0>(rhs);
        _col1 = get<1>(rhs);
        _col2 = get<2>(rhs);
        _col3 = get<3>(rhs);
        return *this;
    }

    /** Create a transformation matrix to translate and uniformly-scale a src_rectangle to a dst_rectangle.
     *
     * The implementation is located in scale.hpp since the definition requires both scale and translate.
     *
     * @param src_rectangle The rectangle to be transformed.
     * @param dst_rectangle The rectangle after transformation.
     * @param alignment How the src_rectangle should be aligned inside the dst_rectangle after scaling and moving.
     * @return A transformation matrix to move and scale the src_rectangle to the dst_rectangle.
     */
    [[nodiscard]] constexpr static matrix uniform(aarectangle src_rectangle, aarectangle dst_rectangle, alignment alignment) noexcept;

    template<int I>
    [[nodiscard]] friend constexpr f32x4 get(matrix const &rhs) noexcept
    {
        if constexpr (I == 0) {
            return rhs._col0;
        } else if constexpr (I == 1) {
            return rhs._col1;
        } else if constexpr (I == 2) {
            return rhs._col2;
        } else if constexpr (I == 3) {
            return rhs._col3;
        } else {
            tt_static_no_default();
        }
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return true;
    }

    [[nodiscard]] constexpr auto operator*(f32x4 const &rhs) const noexcept
    {
        return f32x4{_col0 * rhs.xxxx() + _col1 * rhs.yyyy() + _col2 * rhs.zzzz() + _col3 * rhs.wwww()};
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(vector<E> const &rhs) const noexcept
    {
        tt_axiom(rhs.is_valid());
        return vector<std::max(D, E)>{
            _col0 * static_cast<f32x4>(rhs).xxxx() + _col1 * static_cast<f32x4>(rhs).yyyy() +
            _col2 * static_cast<f32x4>(rhs).zzzz()};
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(extent<E> const &rhs) const noexcept
    {
        tt_axiom(rhs.is_valid());
        return extent<std::max(D, E)>{
            _col0 * static_cast<f32x4>(rhs).xxxx() + _col1 * static_cast<f32x4>(rhs).yyyy() +
            _col2 * static_cast<f32x4>(rhs).zzzz()};
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(point<E> const &rhs) const noexcept
    {
        tt_axiom(rhs.is_valid());
        return point<std::max(D, E)>{
            _col0 * static_cast<f32x4>(rhs).xxxx() + _col1 * static_cast<f32x4>(rhs).yyyy() +
            _col2 * static_cast<f32x4>(rhs).zzzz() + _col3 * static_cast<f32x4>(rhs).wwww()};
    }

    [[nodiscard]] constexpr auto operator*(rectangle const &rhs) const noexcept
    {
        return rectangle{*this * get<0>(rhs), *this * get<1>(rhs), *this * get<2>(rhs), *this * get<3>(rhs)};
    }

    /** Transform a color by a color matrix.
     * The alpha value is not included in the transformation and copied from the input.
     * The color will be correctly transformed if the color matrix includes translation.
     */
    [[nodiscard]] constexpr auto operator*(color const &rhs) const noexcept
    {
        tt_axiom(rhs.is_valid());
        auto r = color{
            _col0 * static_cast<f32x4>(rhs).xxxx() + _col1 * static_cast<f32x4>(rhs).yyyy() +
            _col2 * static_cast<f32x4>(rhs).zzzz() + _col3};

        r.a() = rhs.a();
        return r;
    }

    /** Matrix/Matrix multiplication.
     */
    [[nodiscard]] constexpr auto operator*(matrix<2> const &rhs) const noexcept
    {
        return matrix<D>{*this * get<0>(rhs), *this * get<1>(rhs), *this * get<2>(rhs), *this * get<3>(rhs)};
    }

    /** Matrix/Matrix multiplication.
     */
    [[nodiscard]] constexpr auto operator*(matrix<3> const &rhs) const noexcept
    {
        return matrix<3>{*this * get<0>(rhs), *this * get<1>(rhs), *this * get<2>(rhs), *this * get<3>(rhs)};
    }

    /** Matrix transpose.
     */
    [[nodiscard]] friend constexpr matrix transpose(matrix rhs) noexcept
    {
        auto tmp = transpose(rhs._col0, rhs._col1, rhs._col2, rhs._col3);
        return {std::get<0>(tmp), std::get<1>(tmp), std::get<2>(tmp), std::get<3>(tmp)};
    }

    template<int E>
    [[nodiscard]] constexpr bool operator==(matrix<E> const &rhs) const noexcept
    {
        return _col0 == rhs._col0 && _col1 == rhs._col1 && _col2 == rhs._col2 && _col3 == rhs._col3;
    }

    /** Invert matrix.
     */
    [[nodiscard]] constexpr matrix operator~() const
    {
        //                   rc
        // var s0 : Number = i00 * i11 -
        //                  i10 * i01;
        // var c0 : Number = i20 * i31 -
        //                  i30 * i21;
        ttlet s0c0 = _col0 * _col1.yxwz();

        // var s1 : Number = i00 * i12 -
        //                  i10 * i02;
        // var c1 : Number = i20 * i32 -
        //                  i30 * i22;
        ttlet s1c1 = _col0 * _col2.yxwz();
        ttlet s0c0s1c1 = hsub(s0c0, s1c1);

        // var s2 : Number = i00 * i13 -
        //                  i10 * i03;
        // var c2 : Number = i20 * i33 -
        //                  i30 * i23;
        ttlet s2c2 = _col0 * _col3.yxwz();

        // var s3 : Number = i01 * i12 -
        //                  i11 * i02;
        // var c3 : Number = i21 * i32 -
        //                  i31 * i22;
        ttlet s3c3 = _col1 * _col2.yxwz();
        ttlet s2c2s3c3 = hsub(s2c2, s3c3);

        // var s4 : Number = i01 * i13 -
        //                  i11 * i03;
        // var c4 : Number = i21 * i33 -
        //                  i31 * i23;
        ttlet s4c4 = _col1 * _col3.yxwz();

        // var s5 : Number = i02 * i13 -
        //                  i12 * i03;
        // var c5 : Number = i22 * i33 -
        //                  i32 * i23;
        ttlet s5c5 = _col2 * _col3.yxwz();
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

        ttlet det_prod_half0 = neg<0b0010>(s0123 * c5432);
        ttlet det_prod_half1 = neg<0b0001>(s45__ * c10__);

        ttlet det_sum0 = hadd(det_prod_half0, det_prod_half1);
        ttlet det_sum1 = hadd(det_sum0, det_sum0);
        ttlet det = hadd(det_sum1, det_sum1).xxxx();

        if (det.x() == 0.0f) {
            throw std::domain_error("Divide by zero");
        }

        ttlet invdet = rcp(det);

        ttlet t = transpose(*this);

        //   rc     rc          rc          rc
        // m.i00 = (i11 *  c5 + i12 * -c4 + i13 *  c3) * invdet;
        // m.i10 = (i10 * -c5 + i12 *  c2 + i13 * -c1) * invdet;
        // m.i20 = (i10 *  c4 + i11 * -c2 + i13 *  c0) * invdet;
        // m.i30 = (i10 * -c3 + i11 *  c1 + i12 * -c0) * invdet;
        auto tmp_c5543 = neg<0b1010>(c5432.xxyz());
        auto tmp_c4221 = neg<0b0101>(c5432.yww0() + c10__._000x());
        auto tmp_c3100 = neg<0b1010>(c5432.z000() + c10__._0xyy());
        ttlet inv_col0 = ((t._col1.yxxx() * tmp_c5543) + (t._col1.zzyy() * tmp_c4221) + (t._col1.wwwz() * tmp_c3100)) * invdet;

        // m.i01 = (i01 * -c5 + i02 *  c4 + i03 * -c3) * invdet;
        // m.i11 = (i00 *  c5 + i02 * -c2 + i03 *  c1) * invdet;
        // m.i21 = (i00 * -c4 + i01 *  c2 + i03 * -c0) * invdet;
        // m.i31 = (i00 *  c3 + i01 * -c1 + i02 *  c0) * invdet;
        tmp_c5543 = -tmp_c5543;
        tmp_c4221 = -tmp_c4221;
        tmp_c3100 = -tmp_c3100;
        ttlet inv_col1 = ((t._col0.yxxx() * tmp_c5543) + (t._col0.zzyy() * tmp_c4221) + (t._col0.wwwz() * tmp_c3100)) * invdet;

        // m.i02 = (i31 *  s5 + i32 * -s4 + i33 *  s3) * invdet;
        // m.i12 = (i30 * -s5 + i32 *  s2 + i33 * -s1) * invdet;
        // m.i22 = (i30 *  s4 + i31 * -s2 + i33 *  s0) * invdet;
        // m.i32 = (i30 * -s3 + i31 *  s1 + i32 * -s0) * invdet;
        auto tmp_s5543 = neg<0b1010>(s45__.yyx0() + s0123._000w());
        auto tmp_s4221 = neg<0b0101>(s45__.x000() + s0123._0zzy());
        auto tmp_s3100 = neg<0b1010>(s0123.wyxx());
        ttlet inv_col2 = ((t._col3.yxxx() * tmp_s5543) + (t._col3.zzyy() * tmp_s4221) + (t._col3.wwwz() * tmp_s3100)) * invdet;

        // m.i03 = (i21 * -s5 + i22 *  s4 + i23 * -s3) * invdet;
        // m.i13 = (i20 *  s5 + i22 * -s2 + i23 *  s1) * invdet;
        // m.i23 = (i20 * -s4 + i21 *  s2 + i23 * -s0) * invdet;
        // m.i33 = (i20 *  s3 + i21 * -s1 + i22 *  s0) * invdet;
        tmp_s5543 = -tmp_s5543;
        tmp_s4221 = -tmp_s4221;
        tmp_s3100 = -tmp_s3100;
        ttlet inv_col3 = ((t._col2.yxxx() * tmp_s5543) + (t._col2.zzyy() * tmp_s4221) + (t._col2.wwwz() * tmp_s3100)) * invdet;

        return {inv_col0, inv_col1, inv_col2, inv_col3};
    }

private:
    f32x4 _col0;
    f32x4 _col1;
    f32x4 _col2;
    f32x4 _col3;
};

} // namespace geo

using matrix2 = geo::matrix<2>;
using matrix3 = geo::matrix<3>;

} // namespace tt
