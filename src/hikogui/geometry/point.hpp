// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "vector.hpp"
#include "extent.hpp"
#include "../SIMD/module.hpp"
#include "../utility/module.hpp"
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
template<typename T, int D>
class point {
public:
    using value_type = T;
    using array_type = simd<value_type, 4>;

    static_assert(D == 2 || D == 3, "Only 2D or 3D points are supported");

    constexpr point(point const&) noexcept = default;
    constexpr point(point&&) noexcept = default;
    constexpr point& operator=(point const&) noexcept = default;
    constexpr point& operator=(point&&) noexcept = default;

    /** Construct a point from a lower dimension point.
     */
    template<int E>
        requires(E < D)
    [[nodiscard]] constexpr point(point<value_type, E> const& other) noexcept : _v(static_cast<array_type>(other))
    {
        hi_axiom(holds_invariant());
    }

    /** Construct a point from a lower dimension point.
     */
    [[nodiscard]] constexpr point(point<value_type, 2> const& other, value_type z) noexcept
        requires(D == 3)
        : _v(static_cast<array_type>(other))
    {
        _v.z() = z;
        hi_axiom(holds_invariant());
    }

    /** Construct a point from a higher dimension point.
     * This will clear the values in the higher dimensions.
     */
    template<int E>
        requires(E > D)
    [[nodiscard]] constexpr explicit point(point<value_type, E> const& other) noexcept : _v(static_cast<array_type>(other))
    {
        for (std::size_t i = D; i != E; ++i) {
            _v[i] = value_type{0};
        }
        hi_axiom(holds_invariant());
    }

    /** Convert a point to its array_type-nummeric_array.
     */
    [[nodiscard]] constexpr explicit operator array_type() const noexcept
    {
        hi_axiom(holds_invariant());
        return _v;
    }

    /** Construct a point from a array_type-simd.
     */
    [[nodiscard]] constexpr explicit point(array_type const& other) noexcept : _v(other)
    {
        hi_axiom(holds_invariant());
    }

    /** Construct a point at the origin of the coordinate system.
     */
    [[nodiscard]] constexpr point() noexcept : _v(value_type{0}, value_type{0}, value_type{0}, value_type{1}) {}

    /** Construct a 2D point from x and y elements.
     * @param x The x element.
     * @param y The y element.
     */
    [[nodiscard]] constexpr point(value_type x, value_type y) noexcept
        requires(D == 2)
        : _v(x, y, value_type{0}, value_type{1})
    {
    }

    /** Construct a 3D point from x, y and z elements.
     * @param x The x element.
     * @param y The y element.
     * @param z The z element.
     */
    [[nodiscard]] constexpr point(value_type x, value_type y, value_type z = value_type{0}) noexcept
        requires(D == 3)
        : _v(x, y, z, value_type{1})
    {
    }

    /** Access the x element from the point.
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr value_type& x() noexcept
    {
        return _v.x();
    }

    /** Access the y element from the point.
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr value_type& y() noexcept
    {
        return _v.y();
    }

    /** Access the z element from the point.
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr value_type& z() noexcept
        requires(D == 3)
    {
        return _v.z();
    }

    /** Access the x element from the point.
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr value_type x() const noexcept
    {
        return _v.x();
    }

    /** Access the y element from the point.
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr value_type y() const noexcept
    {
        return _v.y();
    }

    /** Access the z element from the point.
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr value_type z() const noexcept
        requires(D == 3)
    {
        return _v.z();
    }

    template<int E>
        requires(E <= D)
    constexpr point& operator+=(vector<value_type, E> const& rhs) noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        _v = _v + static_cast<array_type>(rhs);
        return *this;
    }

    template<int E>
        requires(E <= D)
    constexpr point& operator-=(vector<value_type, E> const& rhs) noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        _v = _v - static_cast<array_type>(rhs);
        return *this;
    }

    /** Move a point along a vector.
     * @param lhs The point to move.
     * @param rhs The vector to move along.
     * @return The moved point.
     */
    template<int E>
    [[nodiscard]] constexpr friend auto operator+(point const& lhs, vector<value_type, E> const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return point<value_type, std::max(D, E)>{lhs._v + static_cast<array_type>(rhs)};
    }

    /** Move a point along a vector.
     * @param lhs The vector to move along.
     * @param rhs The point to move.
     * @return The moved point.
     */
    template<int E>
    [[nodiscard]] constexpr friend auto operator+(vector<value_type, E> const& rhs, point const& lhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return point<value_type, std::max(D, E)>{lhs._v + static_cast<array_type>(rhs)};
    }

    /** Move a point backward along the vector.
     * @param lhs The point to move.
     * @param rhs The vector to move backward.
     * @return The moved point.
     */
    template<int E>
    [[nodiscard]] constexpr friend auto operator-(point const& lhs, vector<value_type, E> const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return point<value_type, std::max(D, E)>{lhs._v - static_cast<array_type>(rhs)};
    }

    /** Find the vector between two points
     * @param lhs The first point.
     * @param rhs The second point.
     * @return The vector from the second to first point.
     */
    [[nodiscard]] constexpr friend vector<value_type, D> operator-(point const& lhs, point const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return vector<value_type, D>{lhs._v - rhs._v};
    }

    /** Compare if two points are equal.
     * @param lhs The first point.
     * @param rhs The second point.
     * @return True if both point are completely equal to each other.
     */
    [[nodiscard]] constexpr friend bool operator==(point const& lhs, point const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return equal(lhs._v, rhs._v);
    }

    template<int E>
    [[nodiscard]] friend constexpr auto midpoint(point const& lhs, point<value_type, E> const& rhs) noexcept
    {
        return point<value_type, std::max(D, E)>{midpoint(static_cast<array_type>(lhs), static_cast<array_type>(rhs))};
    }

    template<int E>
    [[nodiscard]] friend constexpr auto reflect(point const& lhs, point<value_type, E> const& rhs) noexcept
    {
        return point<value_type, std::max(D, E)>{reflect_point(static_cast<array_type>(lhs), static_cast<array_type>(rhs))};
    }

    /** Mix the two points and get the lowest value of each element.
     * @param lhs The first point.
     * @param rhs The first point.
     * @return A point that is the most left of both points, and most bottom of both points.
     */
    template<int E>
    [[nodiscard]] friend constexpr auto min(point const& lhs, point<value_type, E> const& rhs) noexcept
    {
        return point<value_type, std::max(D, E)>{min(static_cast<array_type>(lhs), static_cast<array_type>(rhs))};
    }

    /** Mix the two points and get the highest value of each element.
     * @param lhs The first point.
     * @param rhs The first point.
     * @return A point that is the most right of both points, and most top of both points.
     */
    template<int E>
    [[nodiscard]] friend constexpr auto max(point const& lhs, point<value_type, E> const& rhs) noexcept
    {
        return point<value_type, std::max(D, E)>{max(static_cast<array_type>(lhs), static_cast<array_type>(rhs))};
    }

    /** Round the coordinates of a point toward nearest integer.
     */
    [[nodiscard]] friend constexpr point round(point const& rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        return point{round(static_cast<array_type>(rhs))};
    }

    /** Round the coordinates of a point toward the right-top.
     */
    [[nodiscard]] friend constexpr point ceil(point const& rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        return point{ceil(static_cast<array_type>(rhs))};
    }

    /** Round the coordinates of a point toward the left-bottom.
     */
    [[nodiscard]] friend constexpr point floor(point const& rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        return point{floor(static_cast<array_type>(rhs))};
    }

    /** Round the coordinates of a point toward the top-right with the given granularity.
     */
    [[nodiscard]] friend constexpr point ceil(point const& lhs, extent<value_type, D> rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        hilet rhs_ = array_type{rhs}.xy11();
        return point{ceil(array_type{lhs} / rhs_) * rhs_};
    }

    /** Round the coordinates of a point toward the top-right with the given granularity.
     */
    [[nodiscard]] friend constexpr point ceil(point const& lhs, extent<value_type, D> rhs) noexcept
        requires std::is_same_v<value_type, int>
    {
        hilet rhs_ = array_type{rhs}.xy11();
        hilet lhs_ = array_type{lhs};
        return point{(lhs_ + (rhs_ - 1)) / rhs_ * rhs_};
    }

    /** Round the coordinates of a point toward the left-bottom with the given granularity.
     */
    [[nodiscard]] friend constexpr point floor(point const& lhs, extent<value_type, D> rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        hilet rhs_ = array_type{rhs}.xy11();
        return point{floor(array_type{lhs} / rhs_) * rhs_};
    }

    /** Round the coordinates of a point toward the left-bottom with the given granularity.
     */
    [[nodiscard]] friend constexpr point floor(point const& lhs, extent<value_type, D> rhs) noexcept
        requires std::is_same_v<value_type, int>
    {
        hilet rhs_ = array_type{rhs}.xy11();
        hilet lhs_ = array_type{lhs};
        return point{lhs_ / rhs_ * rhs_};
    }

    [[nodiscard]] friend constexpr value_type distance(point const& lhs, point const& rhs) noexcept
    {
        return hypot(rhs - lhs);
    }

    /** Check if the point is valid.
     * This function will check if w is not zero, and with a 2D point is z is zero.
     */
    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() != value_type{0} && (D == 3 || _v.z() == value_type{0});
    }

    [[nodiscard]] friend std::string to_string(point const& rhs) noexcept
    {
        if constexpr (D == 2) {
            return std::format("<{}, {}>", rhs._v.x(), rhs._v.y());
        } else if constexpr (D == 3) {
            return std::format("<{}, {}, {}>", rhs._v.x(), rhs._v.y(), rhs._v.z());
        } else {
            hi_static_no_default();
        }
    }

    friend std::ostream& operator<<(std::ostream& lhs, point const& rhs) noexcept
    {
        return lhs << to_string(rhs);
    }

private:
    array_type _v;
};

} // namespace geo

using point2 = geo::point<float, 2>;
using point3 = geo::point<float, 3>;
using point2i = geo::point<int, 2>;
using point3i = geo::point<int, 3>;

template<>
[[nodiscard]] constexpr point2 narrow_cast(point2i const& rhs) noexcept
{
    return {narrow_cast<float>(rhs.x()), narrow_cast<float>(rhs.y())};
}

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::geo::point<float, 2>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::point<float, 2> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "<{}, {}>", std::make_format_args(t.x(), t.y()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::point<float, 3>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::point<float, 3> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "<{}, {}, {}>", std::make_format_args(t.x(), t.y(), t.z()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::point<int, 2>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::point<int, 2> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "<{}, {}>", std::make_format_args(t.x(), t.y()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::point<int, 3>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::point<int, 3> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "<{}, {}, {}>", std::make_format_args(t.x(), t.y(), t.z()));
    }
};
