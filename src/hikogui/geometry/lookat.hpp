// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix.hpp"
#include "point.hpp"

namespace hi { inline namespace v1 {
namespace geo {

/** Perspective transform.
 * @ingroup geometry
 */
class lookat {
public:
    lookat() = delete;
    constexpr lookat(lookat const&) noexcept = default;
    constexpr lookat(lookat&&) noexcept = default;
    constexpr lookat& operator=(lookat const&) noexcept = default;
    constexpr lookat& operator=(lookat&&) noexcept = default;

    constexpr lookat(point3 camera_location, point3 lookat_location, float up_angle = 0.0f) :
        _camera_location(camera_location), _lookat_location(lookat_location), _up_angle(up_angle)
    {
    }

    [[nodiscard]] constexpr operator matrix<3>() noexcept
    {
        hilet forward = normalize(_camera_location - _lookat_location);
        hilet right = cross(vector3{0.0f, 1.0f, 0.0f}, forward);
        hilet up = cross(forward, right);
        hilet lookat_matrix = matrix3{
            static_cast<f32x4>(right), static_cast<f32x4>(up), static_cast<f32x4>(forward), static_cast<f32x4>(_camera_location)};
        return lookat_matrix;
    }

private:
    point3 _camera_location;
    point3 _lookat_location;
    float _up_angle;
};

} // namespace geo

using lookat3 = geo::lookat;

}} // namespace hi::v1
