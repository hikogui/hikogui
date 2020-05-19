// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/aarect.hpp"
#include "TTauri/Foundation/attributes.hpp"
#include <array>

namespace TTauri {

/** Class which represents an axis-aligned rectangle.
 */
class rect {
    /** Intrinsic of the rectangle.
     */
    std::array<vec,4> corners;

public:
    force_inline rect() noexcept : corners({vec{}, vec{}, vec{}, vec{}}) {}
    force_inline rect(rect const &rhs) noexcept = default;
    force_inline rect &operator=(rect const &rhs) noexcept = default;
    force_inline rect(rect &&rhs) noexcept = default;
    force_inline rect &operator=(rect &&rhs) noexcept = default;

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
        ttauri_assume(corner0.is_point());
        ttauri_assume(extent.is_vector());
        ttauri_assume(extent.z() == 0.0);
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

    /** Get coordinate of a corner.
    *
    * @param I Corner number: 0 = left-bottom, 1 = right-bottom, 2 = left-top, 3 = right-top.
    * @return The homogeneous coordinate of the corner.
    */
    template<size_t I>
    [[nodiscard]] force_inline vec corner() const noexcept {
        static_assert(I <= 3);
        return get<I>(corners);
    }

    [[nodiscard]] friend rect expand(rect const &lhs, float rhs) noexcept {
        let rightDirection = normalize(lhs.right());
        let upDirection = normalize(lhs.up());

        return {
            lhs.corner<0>() + rhs * -rightDirection + rhs * -upDirection,
            lhs.corner<1>() + rhs *  rightDirection + rhs * -upDirection,
            lhs.corner<2>() + rhs * -rightDirection + rhs *  upDirection,
            lhs.corner<3>() + rhs *  rightDirection + rhs *  upDirection
        };
    }

    /** Round the rectangle to have its corners in the center of a pixel.
     * The extent of the rectangle should be maintained as closely as possible.
     *
     * The center of a pixel is at integer value.
     */
    template<bool ToCorner=false>
    [[nodiscard]] friend rect round2D(rect const &rhs) noexcept {
        let corner0 = rhs.corner<0>();
        let h_vec = rhs.corner<1>() - corner0;
        let v_vec = rhs.corner<2>() - corner0;

        let rounded_corner0 = round2D<ToCorner>(corner0);
        
        let corner1 = rounded_corner0 + h_vec;
        let corner2 = rounded_corner0 + v_vec;
        let corner3 = corner1 + v_vec;

        rect r;
        get<0>(r.corners) = rounded_corner0;
        get<1>(r.corners) = round2D<ToCorner>(corner1);
        get<2>(r.corners) = round2D<ToCorner>(corner2);
        get<3>(r.corners) = round2D<ToCorner>(corner3);
        return r;
    }


};

}

