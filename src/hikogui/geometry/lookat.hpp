// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file geometry/lookat.hpp Defines lookat
 * @ingroup geometry
 */

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

    constexpr lookat(point3 camera_location, point3 lookat_location, vector3 up = vector3{0.0f, 1.0f, 0.0f}) :
        _camera_location(camera_location), _lookat_location(lookat_location), _up(up)
    {
    }

    [[nodiscard]] constexpr operator matrix<3>() noexcept
    {
        hilet f = normalize(_lookat_location - _camera_location);
        hilet s = normalize(cross(f, _up));
        hilet u = cross(s, f);

        hilet eye = vector3{static_cast<f32x4>(_camera_location).xyz0()};

        // clang-format off
        // Matrix constructor is in row-major for nice display.
        return matrix3{
             s.x(),  u.x(), -f.x(), -dot(s, eye),
             s.y(),  u.y(), -f.y(), -dot(u, eye),
             s.z(),  u.z(), -f.z(), -dot(f, eye),
             0.0f ,  0.0f ,  0.0f ,  1.0f
        };
        // clang-format on
    }

private:
    point3 _camera_location;
    point3 _lookat_location;
    vector3 _up;
};

} // namespace geo

using lookat3 = geo::lookat;

}} // namespace hi::v1
