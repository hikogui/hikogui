// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "numeric_array.hpp"
#include "aarect.hpp"
#include "alignment.hpp"
#include <array>

namespace tt {

/** Class which represents an rectangle.
 */
class rect {
    /** Intrinsic of the rectangle.
     */
    std::array<f32x4,4> corners;

public:
    rect() noexcept : corners({f32x4{}, f32x4{}, f32x4{}, f32x4{}}) {}
    rect(rect const &rhs) noexcept = default;
    rect &operator=(rect const &rhs) noexcept = default;
    rect(rect &&rhs) noexcept = default;
    rect &operator=(rect &&rhs) noexcept = default;

    rect(f32x4 corner0, f32x4 corner1, f32x4 corner2, f32x4 corner3) noexcept :
        corners({corner0, corner1, corner2, corner3}) {}

    rect(aarect rhs) noexcept :
        corners({rhs.corner<0>(), rhs.corner<1>(), rhs.corner<2>(), rhs.corner<3>()}) {}

    rect &operator=(aarect rhs) noexcept {
        corners[0] = rhs.corner<0>();
        corners[1] = rhs.corner<1>();
        corners[2] = rhs.corner<2>();
        corners[3] = rhs.corner<3>();
        return *this;
    }

    rect(f32x4 corner0, f32x4 extent) noexcept :
        corners({
            corner0,
            corner0 + extent.x000(),
            corner0 + extent._0y00(),
            corner0 + extent.xy00()
        })
    {
        tt_assume(corner0.is_point());
        tt_assume(extent.is_vector());
        tt_assume(extent.z() == 0.0);
    }

    /** Get the right vector of a rectangle.
     */
    f32x4 right_vector() const noexcept {
        return corner<1>() - corner<0>();
    }

    /** Get the up vector of a rectangle.
    */
    f32x4 up_vector() const noexcept {
        return corner<2>() - corner<0>();
    }

    float width() const noexcept {
        return length<3>(right_vector());
    }

    float height() const noexcept {
        return length<3>(up_vector());
    }

    f32x4 extent() const noexcept {
        return {width(), height()};
    }

    aarect aabb() const noexcept {
        // XXX - Should actually check maximum and minimums of all points.
        return aarect::p0p3(std::get<0>(corners).xy01(), std::get<3>(corners).xy01());
    }

    /** Get coordinate of a corner.
    *
    * @param I Corner number: 0 = left-bottom, 1 = right-bottom, 2 = left-top, 3 = right-top.
    * @return The homogeneous coordinate of the corner.
    */
    template<size_t I>
    [[nodiscard]] f32x4 corner() const noexcept {
        static_assert(I <= 3);
        return std::get<I>(corners);
    }

    [[nodiscard]] friend rect expand(rect const &lhs, float rhs) noexcept {
        ttlet rightDirection = normalize<3>(lhs.right_vector());
        ttlet upDirection = normalize<3>(lhs.up_vector());

        return {
            lhs.corner<0>() + rhs * -rightDirection + rhs * -upDirection,
            lhs.corner<1>() + rhs *  rightDirection + rhs * -upDirection,
            lhs.corner<2>() + rhs * -rightDirection + rhs *  upDirection,
            lhs.corner<3>() + rhs *  rightDirection + rhs *  upDirection
        };
    }
};

}

