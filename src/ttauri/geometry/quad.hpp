

#pragma once

#include "vector.hpp"
#include "point.hpp"
#include "extent.hpp"
#include "axis_aligned_rectangle.hpp"
#include "rectangle.hpp"

namespace tt {

class quad {
public:
    point3 p0; ///< Left-bottom
    point3 p1; ///< Right-bottom
    point3 p2; ///< Left-top
    point3 p3; ///< Right-top

    constexpr quad() noexcept : p0(), p1(), p2(), p3() {}

    constexpr quad(point3 p0, point3 p1, point3 p2, point3 p3) noexcept : p0(p0), p1(p1), p2(p2), p3(p3) {}

    constexpr quad(quad const &) noexcept = default;
    constexpr quad(quad &&) noexcept = default;
    constexpr quad &operator=(quad const &) noexcept = default;
    constexpr quad &operator=(quad &&) noexcept = default;

    constexpr quad(rectangle const &rhs) noexcept : p0(get<0>(rhs)), p1(get<1>(rhs)), p2(get<2>(rhs)), p3(get<3>(rhs)) {}

    /** The vector from left-bottom to right-bottom.
     */
    [[nodiscard]] constexpr vector3 bottom() const noexcept
    {
        return p1 - p0;
    }

    /** The vector from left-top to right-top.
     */
    [[nodiscard]] constexpr vector3 top() const noexcept
    {
        return p3 - p2;
    }

    /** The vector from left-bottom to left-top.
     */
    [[nodiscard]] constexpr vector3 left() const noexcept
    {
        return p2 - p0;
    }

    /** The vector from right-bottom to right-top.
     */
    [[nodiscard]] constexpr vector3 right() const noexcept
    {
        return p3 - p1;
    }

    /** Add a border around the quad.
     *
     * Move each corner of the quad by the given size outward in the direction of the edges.
     *
     * @param lhs A quad.
     * @param rhs The width and height to add to each corner of the quad.
     * @return The new quad extended by the size.
     */
    [[nodiscard]] friend constexpr quad operator+(quad const &lhs, extent2 const &rhs) noexcept
    {
        ttlet top_extra = normalize(lhs.top()) * rhs.width();
        ttlet bottom_extra = normalize(lhs.bottom()) * rhs.width();
        ttlet left_extra = normalize(lhs.left()) * rhs.height();
        ttlet right_extra = normalize(lhs.right()) * rhs.height();

        return {
            lhs.p0 - bottom_extra - left_extra,
            lhs.p1 + bottom_extra - right_extra,
            lhs.p2 - top_extra + left_extra,
            lhs.p3 + top_extra + right_extra};
    }

    [[nodiscard]] friend constexpr aarectangle bounding_rectangle(quad const &rhs) noexcept
    {
        auto min_p = rhs.p0;
        auto max_p = rhs.p0;

        min_p = min(min_p, rhs.p1);
        max_p = max(max_p, rhs.p1);
        min_p = min(min_p, rhs.p2);
        max_p = max(max_p, rhs.p2);
        min_p = min(min_p, rhs.p3);
        max_p = max(max_p, rhs.p3);
        return aarectangle{point2{min_p}, point2{max_p}};
    }

    constexpr quad &operator+=(extent2 const &rhs) noexcept
    {
        return *this = *this + rhs;
    }

    [[nodiscard]] friend constexpr bool operator==(quad const &lhs, quad const &rhs) noexcept = default;
};

} // namespace tt
