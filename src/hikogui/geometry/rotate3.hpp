// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix3.hpp"
#include "identity.hpp"

namespace hi { inline namespace v1 {

class rotate3 {
public:
    using array_type = simd<float, 4>;
    using value_type = array_type::value_type;

    constexpr rotate3(rotate3 const&) noexcept = default;
    constexpr rotate3(rotate3&&) noexcept = default;
    constexpr rotate3& operator=(rotate3 const&) noexcept = default;
    constexpr rotate3& operator=(rotate3&&) noexcept = default;
    constexpr rotate3() noexcept : _v(0.0f, 0.0f, 1.0f, 0.0f) {}

    [[nodiscard]] rotate3(float angle, vector3 axis) noexcept : _v()
    {
        hi_axiom(axis.holds_invariant());
        hi_axiom(std::abs(hypot(axis) - 1.0f) < 0.0001f);

        hilet half_angle = angle * 0.5f;
        hilet C = std::cos(half_angle);
        hilet S = std::sin(half_angle);

        _v = static_cast<f32x4>(axis) * S;
        _v.w() = C;
    }

    /** Convert quaternion to matrix.
     *
     */
    [[nodiscard]] constexpr operator matrix3() const noexcept
    {
        // Original from https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
        //   1 - 2(yy + zz) |     2(xy - zw) |     2(xz + yw)
        //       2(xy + zw) | 1 - 2(xx + zz) |     2(yz - xw)
        //       2(xz - yw) |     2(yz + xw) | 1 - 2(xx + yy)

        // Flipping adds and multiplies:
        //   1 - 2(zz + yy) |     2(xy - zw) |     2(yw + xz)
        //       2(zw + yx) | 1 - 2(xx + zz) |     2(yz - xw)
        //       2(zx - yw) |     2(xw + zy) | 1 - 2(yy + xx)

        // All multiplies.
        hilet x_mul = _v.xxxx() * _v;
        hilet y_mul = _v.yyyy() * _v;
        hilet z_mul = _v.zzzz() * _v;

        auto twos = f32x4(-2.0f, 2.0f, 2.0f, 0.0f);
        auto one = f32x4(1.0f, 0.0f, 0.0f, 0.0f);
        hilet col0 = one + addsub<0b0011>(z_mul.zwxy(), y_mul.yxwz()) * twos;
        one = one.yxzw();
        twos = twos.yxzw();
        hilet col1 = one + addsub<0b0110>(x_mul.yxwz(), z_mul.wzyx()) * twos;
        one = one.xzyw();
        twos = twos.xzyw();
        hilet col2 = one + addsub<0b0101>(y_mul.wzyx(), x_mul.zwxy()) * twos;
        one = one.xywz();
        return matrix3{col0, col1, col2, one};
    }

    

    //std::pair<float, vector3> angle_and_axis() const noexcept
    //{
    //    hilet rcp_length = rcp_hypot<0b0111>(_v);
    //    hilet length = 1.0f / rcp_length;
    //
    //    return {2.0f * std::atan2(length), vector3{_v.xyz0() * rcp_length}};
    //}

private:
    /** rotation is stored as a quaternion
     * w + x*i + y*j + z*k
     */
    array_type _v;
};

}} // namespace hi::v1
