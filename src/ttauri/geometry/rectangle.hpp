// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "numeric_array.hpp"
#include "axis_aligned_rectangle.hpp"
#include "../alignment.hpp"
#include <array>

namespace tt {

/** Class which represents an rectangle.
 */
class rectangle {
    /** Intrinsic of the rectangle.
     */
    std::array<point3, 4> corners;

public:
    rectangle() noexcept : corners{} {}
    rectangle(rectangle const &rhs) noexcept = default;
    rectangle &operator=(rectangle const &rhs) noexcept = default;
    rectangle(rectangle &&rhs) noexcept = default;
    rectangle &operator=(rectangle &&rhs) noexcept = default;

    rectangle(point3 corner0, point3 corner1, point3 corner2, point3 corner3) noexcept : corners{corner0, corner1, corner2, corner3} {}

    rectangle(aarectangle rhs) noexcept : corners{get<0>(rhs), get<1>(rhs), get<2>(rhs), get<3>(rhs)} {}

    rectangle &operator=(aarectangle rhs) noexcept
    {
        std::get<0>(corners) = get<0>(rhs);
        std::get<1>(corners) = get<1>(rhs);
        std::get<2>(corners) = get<2>(rhs);
        std::get<3>(corners) = get<3>(rhs);
        return *this;
    }

    rectangle(point3 corner0, extent2 extent) noexcept :
        corners{corner0, corner0 + extent.right(), corner0 + extent.up(), corner0 + extent.right() + extent.up()}
    {
    }

    [[nodiscard]] explicit operator aarectangle() const noexcept
    {
        // XXX - Should actually check maximum and minimums of all points.
        return aarectangle{point2{std::get<0>(corners)}, point2{std::get<3>(corners)}};
    }

    /** Get the right vector of a rectangle.
     */
    vector3 right_vector() const noexcept
    {
        return std::get<1>(corners) - std::get<0>(corners);
    }

    /** Get the up vector of a rectangle.
     */
    vector3 up_vector() const noexcept
    {
        return std::get<2>(corners) - std::get<0>(corners);
    }

    float width() const noexcept
    {
        return hypot(right_vector());
    }

    float height() const noexcept
    {
        return hypot(up_vector());
    }

    extent2 extent() const noexcept
    {
        return {width(), height()};
    }

    [[nodiscard]] point3 constexpr operator[](size_t i) const noexcept
    {
        tt_axiom(i < 4);
        return corners[i];
    }

    template<size_t I>
    [[nodiscard]] friend constexpr point3 get(rectangle const &rhs) noexcept
    {
        static_assert(I < 4);
        return std::get<I>(rhs.corners);
    }

    [[nodiscard]] friend rectangle expand(rectangle const &lhs, float rhs) noexcept
    {
        ttlet rightDirection = normalize(lhs.right_vector());
        ttlet upDirection = normalize(lhs.up_vector());

        return {
            get<0>(lhs) + rhs * -rightDirection + rhs * -upDirection,
            get<1>(lhs) + rhs * rightDirection + rhs * -upDirection,
            get<2>(lhs) + rhs * -rightDirection + rhs * upDirection,
            get<3>(lhs) + rhs * rightDirection + rhs * upDirection};
    }

    [[nodiscard]] friend rectangle shrink(rectangle const &lhs, float rhs) noexcept
    {
        return expand(lhs, -rhs);
    }
};

} // namespace tt
