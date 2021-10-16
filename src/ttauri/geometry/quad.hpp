

#pragma once

#include "vector.hpp"
#include "point.hpp"
#include "extent.hpp"

namespace tt {

class quad {
public:
    point3 p0; ///< Left-bottom
    point3 p1; ///< Right-bottom
    point3 p2; ///< Left-top
    point3 p3; ///< Right-top

    constexpr quad() noexcept :
        p0(), p1(), p2(), p3() {}

    constexpr quad(point3 p0, point3 p1, point3 p2, point3 p3) noexcept :
        p0(p0), p1(p1), p2(p2), p3(p3) {}

    constexpr quad(quad const &) noexcept = default;
    constexpr quad(quad &&) noexcept = default;
    constexpr quad &operator=(quad const &) noexcept = default;
    constexpr quad &operator=(quad &&) noexcept = default;

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
    [[nodiscard]] friend constexpr quad operator+(quad const &lhs, extent2 const &rhs) const noexcept
    {
        ttlet top_extra = normalize(top()) * rhs.width();
        ttlet bottom_extra = normalize(bottom()) * rhs.width();
        ttlet left_extra = normalize(left()) * rhs.height();
        ttlet right_extra = normalize(right()) * rhs.height();

        return {
            p0 - bottom_extra - left_extra,
            p1 + bottom_extra - right_extra,
            p2 - top_extra + left_extra,
            p3 + top_extra + right_extra
        };
    }

    /** scale the quad.
     *
     * Each edge of the quad scaled.
     *
     * @param lhs A quad.
     * @param rhs The width and height to scale each edge with.
     * @return The new quad extended by the size.
     */
    [[nodiscard]] friend constexpr quad operator*(quad const &lhs, extent2 const &rhs) const noexcept
    {
        ttlet top_extra = (top() * rhs.width() - top()) * 0.5f;
        ttlet bottom_extra = (bottom() * rhs.width() - bottom()) * 0.5f;
        ttlet left_extra = (left() * rhs.height() - left()) * 0.5f;
        ttlet right_extra = (right() * rhs.height() - right()) * 0.5f;

        return {
            p0 - bottom_extra - left_extra,
            p1 + bottom_extra - right_extra,
            p2 - top_extra + left_extra,
            p3 + top_extra + right_extra
        };
    }

    constexpr quad &operator+=(extent2 const &rhs) const noexcept
    {
        return *this = *this + rhs;
    }

    constexpr quad &operator*=(extent2 const &rhs) const noexcept
    {
        return *this = *this * rhs;
    }

    [[nodiscard]] friend constexpr bool operator==(quad const &lhs, quad const &rhs) noexcept = default;
};

