// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix.hpp"

namespace hi { inline namespace v1 {
namespace geo {

/** Perspective transform.
 * @ingroup geometry
 */
class perspective {
public:
    perspective() = delete;
    constexpr perspective(perspective const&) noexcept = default;
    constexpr perspective(perspective&&) noexcept = default;
    constexpr perspective& operator=(perspective const&) noexcept = default;
    constexpr perspective& operator=(perspective&&) noexcept = default;

    constexpr perspective(float focal_length, float rcpt_aspect, float znear, float zfar) :
        _focal_length(focal_length), _rcpt_aspect(rcpt_aspect), _znear(znear), _zfar(zfar)
    {
    }

    /** Create a right-handed perspective transform.
     *
     * @param fov The field of view in radians.
     * @param view_port The view-port.
     * @param znear The location of the near plane.
     * @param zfar The location of the far plane.
     */
    perspective(float fov, aarectangle view_port, float znear, float zfar) noexcept :
        perspective(1.0f / std::tan(0.5f * fov), view_port.height() / view_port.width(), znear, zfar)
    {
    }

    [[nodiscard]] constexpr operator matrix<3>() noexcept
    {
        auto r = matrix<3>{};
        get<0>(r).x() = _focal_length * _rcpt_aspect;
        get<1>(r).y() = _focal_length;
        get<2>(r).z() = _zfar / (_znear - _zfar);
        get<2>(r).w() = -1.0f;
        get<3>(r).z() = -(_zfar * _znear) / (_zfar - _znear);
        return r;
    }

private:
    float _focal_length;
    float _rcpt_aspect;
    float _znear;
    float _zfar;
};

} // namespace geo

using perspective3 = geo::perspective;

}} // namespace hi::v1
