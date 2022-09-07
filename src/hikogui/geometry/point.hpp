// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../rapid/numeric_array.hpp"
#include "vector.hpp"
#include "extent.hpp"
#include <format>

namespace hi::inline v1 {
namespace geo {

/** A high-level geometric point
 * Part of the high-level vec, point, mat and color types.
 *
 * A point, for both 2D or 3D is internally represented
 * as a 4D homogeneous vector. Which can be efficiently implemented
 * as a __m128 SSE register.
 */
template<int D>
class point {
public:
    static_assert(D == 2 || D == 3, "Only 2D or 3D points are supported");

    constexpr point(point const &) noexcept = default;
    constexpr point(point &&) noexcept = default;
    constexpr point &operator=(point const &) noexcept = default;
    constexpr point &operator=(point &&) noexcept = default;

    /** Construct a point from a lower dimension point.
     */
    template<int E>
    requires(E < D) [[nodiscard]] constexpr point(point<E> const &other) noexcept : _v(static_cast<f32x4>(other))
    {
        hi_axiom(holds_invariant());
    }

    /** Construct a point from a lower dimension point.
     */
    [[nodiscard]] constexpr point(point<2> const &other, float z) noexcept requires(D == 3) : _v(static_cast<f32x4>(other))
    {
        _v.z() = z;
        hi_axiom(holds_invariant());
    }


    /** Construct a point from a higher dimension point.
     * This will clear the values in the higher dimensions.
     */
    template<int E>
    requires(E > D) [[nodiscard]] constexpr explicit point(point<E> const &other) noexcept : _v(static_cast<f32x4>(other))
    {
        for (std::size_t i = D; i != E; ++i) {
            _v[i] = 0.0f;
        }
        hi_axiom(holds_invariant());
    }

    /** Convert a point to its f32x4-nummeric_array.
     */
    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        hi_axiom(holds_invariant());
        return _v;
    }

    /** Construct a point from a f32x4-numeric_array.
     */
    [[nodiscard]] constexpr explicit point(f32x4 const &other) noexcept : _v(other)
    {
        hi_axiom(holds_invariant());
    }

    /** Construct a point at the origin of the coordinate system.
     */
    [[nodiscard]] constexpr point() noexcept : _v(0.0, 0.0, 0.0, 1.0) {}

    /** Construct a 2D point from x and y elements.
     * @param x The x element.
     * @param y The y element.
     */
    [[nodiscard]] constexpr point(float x, float y) noexcept requires(D == 2) : _v(x, y, 0.0, 1.0) {}

    /** Construct a 3D point from x, y and z elements.
     * @param x The x element.
     * @param y The y element.
     * @param z The z element.
     */
    [[nodiscard]] constexpr point(float x, float y, float z = 0.0) noexcept requires(D == 3) : _v(x, y, z, 1.0) {}

    /** Access the x element from the point.
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr float &x() noexcept
    {
        return _v.x();
    }

    /** Access the y element from the point.
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr float &y() noexcept
    {
        return _v.y();
    }

    /** Access the z element from the point.
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr float &z() noexcept requires(D == 3)
    {
        return _v.z();
    }

    /** Access the x element from the point.
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr float const &x() const noexcept
    {
        return _v.x();
    }

    /** Access the y element from the point.
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr float const &y() const noexcept
    {
        return _v.y();
    }

    /** Access the z element from the point.
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr float const &z() const noexcept requires(D == 3)
    {
        return _v.z();
    }

    template<int E>
    requires(E <= D) constexpr point &operator+=(vector<E> const &rhs) noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        _v = _v + static_cast<f32x4>(rhs);
        return *this;
    }

    template<int E>
    requires(E <= D) constexpr point &operator-=(vector<E> const &rhs) noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        _v = _v - static_cast<f32x4>(rhs);
        return *this;
    }

    /** Move a point along a vector.
     * @param lhs The point to move.
     * @param rhs The vector to move along.
     * @return The moved point.
     */
    template<int E>
    [[nodiscard]] constexpr friend auto operator+(point const &lhs, vector<E> const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return point<std::max(D, E)>{lhs._v + static_cast<f32x4>(rhs)};
    }

    /** Move a point along a vector.
     * @param lhs The vector to move along.
     * @param rhs The point to move.
     * @return The moved point.
     */
    template<int E>
    [[nodiscard]] constexpr friend auto operator+(vector<E> const &rhs, point const &lhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return point<std::max(D, E)>{lhs._v + static_cast<f32x4>(rhs)};
    }

    /** Move a point backward along the vector.
     * @param lhs The point to move.
     * @param rhs The vector to move backward.
     * @return The moved point.
     */
    template<int E>
    [[nodiscard]] constexpr friend auto operator-(point const &lhs, vector<E> const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return point<std::max(D, E)>{lhs._v - static_cast<f32x4>(rhs)};
    }

    /** Find the vector between two points
     * @param lhs The first point.
     * @param rhs The second point.
     * @return The vector from the second to first point.
     */
    [[nodiscard]] constexpr friend vector<D> operator-(point const &lhs, point const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return vector<D>{lhs._v - rhs._v};
    }

    /** Compare if two points are equal.
     * @param lhs The first point.
     * @param rhs The second point.
     * @return True if both point are completely equal to each other.
     */
    [[nodiscard]] constexpr friend bool operator==(point const &lhs, point const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return lhs._v == rhs._v;
    }

    template<int E>
    [[nodiscard]] friend constexpr auto midpoint(point const &lhs, point<E> const &rhs) noexcept
    {
        return point<std::max(D, E)>{midpoint(static_cast<f32x4>(lhs), static_cast<f32x4>(rhs))};
    }

    template<int E>
    [[nodiscard]] friend constexpr auto reflect(point const &lhs, point<E> const &rhs) noexcept
    {
        return point<std::max(D, E)>{reflect_point(static_cast<f32x4>(lhs), static_cast<f32x4>(rhs))};
    }

    /** Mix the two points and get the lowest value of each element.
     * @param lhs The first point.
     * @param rhs The first point.
     * @return A point that is the most left of both points, and most bottom of both points.
     */
    template<int E>
    [[nodiscard]] friend constexpr auto min(point const &lhs, point<E> const &rhs) noexcept
    {
        return point<std::max(D, E)>{min(static_cast<f32x4>(lhs), static_cast<f32x4>(rhs))};
    }

    /** Mix the two points and get the heighest value of each element.
     * @param lhs The first point.
     * @param rhs The first point.
     * @return A point that is the most right of both points, and most top of both points.
     */
    template<int E>
    [[nodiscard]] friend constexpr auto max(point const &lhs, point<E> const &rhs) noexcept
    {
        return point<std::max(D, E)>{max(static_cast<f32x4>(lhs), static_cast<f32x4>(rhs))};
    }

    /** Round the coordinates of a point toward nearest integer.
     */
    [[nodiscard]] friend constexpr point round(point const &rhs) noexcept
    {
        return point{round(static_cast<f32x4>(rhs))};
    }

    /** Round the coordinates of a point toward the right-top.
     */
    [[nodiscard]] friend constexpr point ceil(point const &rhs) noexcept
    {
        return point{ceil(static_cast<f32x4>(rhs))};
    }

    /** Round the coordinates of a point toward the left-bottom.
     */
    [[nodiscard]] friend constexpr point floor(point const &rhs) noexcept
    {
        return point{floor(static_cast<f32x4>(rhs))};
    }

    /** Round the coordinates of a point toward the top-right with the given granularity.
     */
    [[nodiscard]] friend constexpr point ceil(point const &lhs, extent2 rhs) noexcept
    {
        hilet rhs_ = f32x4{rhs}.xy11();
        return point{ceil(f32x4{lhs} / rhs_) * rhs_};
    }

    /** Round the coordinates of a point toward the left-bottom with the given granularity.
     */
    [[nodiscard]] friend constexpr point floor(point const &lhs, extent2 rhs) noexcept
    {
        hilet rhs_ = f32x4{rhs}.xy11();
        return point{floor(f32x4{lhs} / rhs_) * rhs_};
    }

    [[nodiscard]] friend constexpr float distance(point const &lhs, point const &rhs) noexcept
    {
        return hypot(rhs - lhs);
    }

    /** Check if the point is valid.
     * This function will check if w is not zero, and with a 2D point is z is zero.
     */
    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() != 0.0f && (D == 3 || _v.z() == 0.0f);
    }

    [[nodiscard]] friend std::string to_string(point const &rhs) noexcept
    {
        if constexpr (D == 2) {
            return std::format("<{}, {}>", rhs._v.x(), rhs._v.y());
        } else if constexpr (D == 3) {
            return std::format("<{}, {}, {}>", rhs._v.x(), rhs._v.y(), rhs._v.z());
        } else {
            hi_static_no_default();
        }
    }

    friend std::ostream &operator<<(std::ostream &lhs, point const &rhs) noexcept
    {
        return lhs << to_string(rhs);
    }

private:
    f32x4 _v;
};

} // namespace geo

using point2 = geo::point<2>;
using point3 = geo::point<3>;

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::geo::point<2>, CharT> {
    auto parse(auto &pc)
    {
        return pc.end();
    }

    auto format(hi::geo::point<2> const &t, auto &fc)
    {
        return std::vformat_to(fc.out(), "<{}, {}>", std::make_format_args(t.x(), t.y()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::point<3>, CharT> : std::formatter<float, CharT> {
    auto parse(auto &pc)
    {
        return pc.end();
    }

    auto format(hi::geo::point<3> const &t, auto &fc)
    {
        return std::vformat_to(fc.out(), "<{}, {}, {}>", std::make_format_args(t.x(), t.y(), t.z()));
    }
};
