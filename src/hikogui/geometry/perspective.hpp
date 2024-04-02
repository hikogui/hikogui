// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix3.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <exception>
#include <compare>
#include <cmath>

hi_export_module(hikogui.geometry : perspective);

hi_export namespace hi { inline namespace v1 {

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

    /** Create a right-handed perspective transform.
     *
     * @note: This makes a right handed perspective matrix, where the near and far plane are clamped between 1.0 and 0.0
     * @param fov_y The field of view from the eye to the height of the view in radians.
     * @param aspect_ratio The view-port.
     * @param znear The distance from the camera to the near plane.
     * @param zfar The distance from the camera to the far plane.
     */
    perspective(float fov_y, float aspect_ratio, float znear, float zfar) noexcept :
        _tan_half_fov_y(std::tan(fov_y * 0.5f)), _aspect_ratio(aspect_ratio), _znear(znear), _zfar(zfar)
    {
        hi_axiom(fov_y > std::numeric_limits<float>::epsilon());
        hi_axiom(aspect_ratio > std::numeric_limits<float>::epsilon());
    }

    /** Create a right-handed perspective transform.
     *
     * @note: This makes a right handed perspective matrix, where the near and far plane are clamped between 1.0 and 0.0
     * @param fov_y The field of view from the eye to the height of the view in radians.
     * @param view_port The size of the view port.
     * @param znear The distance from the camera to the near plane.
     * @param zfar The distance from the camera to the far plane.
     */
    perspective(float fov_y, extent2 view_port, float znear, float zfar) noexcept :
        perspective(fov_y, view_port.width() / view_port.height(), znear, zfar)
    {
    }

    [[nodiscard]] constexpr operator matrix3() noexcept
    {
        auto const a = _aspect_ratio;
        auto const t = _tan_half_fov_y;
        auto const f = _zfar;
        auto const n = _znear;

        // clang-format off
        return {
            1.0f / (a * t), 0.0f    ,  0.0f       ,  0.0f,
            0.0f          , 1.0f / t,  0.0f       ,  0.0f,
            0.0f          , 0.0f    ,  f / (n - f), -(f * n) / (f - n),
            0.0f          , 0.0f    , -1.0f       ,  0.0f
        };
        // clang-format on
    }

private:
    float _tan_half_fov_y;
    float _aspect_ratio;
    float _znear;
    float _zfar;
};

}} // namespace hi::v1
