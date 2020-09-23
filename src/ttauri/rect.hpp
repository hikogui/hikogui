// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "vec.hpp"
#include "aarect.hpp"
#include "attributes.hpp"
#include <array>

namespace tt {

/** Class which represents an axis-aligned rectangle.
 */
class rect {
    /** Intrinsic of the rectangle.
     */
    std::array<vec,4> corners;

public:
    rect() noexcept : corners({vec{}, vec{}, vec{}, vec{}}) {}
    rect(rect const &rhs) noexcept = default;
    rect &operator=(rect const &rhs) noexcept = default;
    rect(rect &&rhs) noexcept = default;
    rect &operator=(rect &&rhs) noexcept = default;

    rect(vec corner0, vec corner1, vec corner2, vec corner3) noexcept :
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

    rect(vec corner0, vec extent) noexcept :
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
    vec right() const noexcept {
        return corner<1>() - corner<0>();
    }

    /** Get the up vector of a rectangle.
    */
    vec up() const noexcept {
        return corner<2>() - corner<0>();
    }

    float width() const noexcept {
        return length(right());
    }

    float height() const noexcept {
        return length(up());
    }

    vec extent() const noexcept {
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
    [[nodiscard]] vec corner() const noexcept {
        static_assert(I <= 3);
        return std::get<I>(corners);
    }

    [[nodiscard]] friend rect expand(rect const &lhs, float rhs) noexcept {
        ttlet rightDirection = normalize(lhs.right());
        ttlet upDirection = normalize(lhs.up());

        return {
            lhs.corner<0>() + rhs * -rightDirection + rhs * -upDirection,
            lhs.corner<1>() + rhs *  rightDirection + rhs * -upDirection,
            lhs.corner<2>() + rhs * -rightDirection + rhs *  upDirection,
            lhs.corner<3>() + rhs *  rightDirection + rhs *  upDirection
        };
    }
};

}

