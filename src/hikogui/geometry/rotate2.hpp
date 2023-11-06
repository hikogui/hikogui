// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../SIMD/SIMD.hpp"
#include "../macros.hpp"
#include <cmath>
#include <exception>
#include <compare>

hi_export_module(hikogui.geometry : rotate2);

hi_export namespace hi { inline namespace v1 {

class rotate2 {
public:
    using array_type = simd<float, 4>;
    using value_type = array_type::value_type;

    constexpr rotate2(rotate2 const&) noexcept = default;
    constexpr rotate2(rotate2&&) noexcept = default;
    constexpr rotate2& operator=(rotate2 const&) noexcept = default;
    constexpr rotate2& operator=(rotate2&&) noexcept = default;
    constexpr rotate2() noexcept : _v(0.0f, 0.0f, 1.0f, 0.0f) {}

    [[nodiscard]] rotate2(float angle) noexcept : _v()
    {
        hilet half_angle = angle * 0.5f;
        hilet C = std::cos(half_angle);
        hilet S = std::sin(half_angle);

        _v = f32x4{0.0f, 0.0f, 1.0f, 0.0f} * S;
        _v.w() = C;
    }

    [[nodiscard]] constexpr explicit operator array_type() const noexcept
    {
        return _v;
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
