// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../SIMD/module.hpp"

namespace hi::inline v1 {

namespace geo {

/** A high-level geometric vector
 * Part of the high-level vector, point, mat and color types.
 *
 * A vector, for both 2D or 3D is internally represented
 * as a 4D homogeneous vector. Which can be efficiently implemented
 * as a __m128 SSE register.
 */
template<typename T, int D>
class vector {
public:
    using value_type = T;
    using array_type = simd<value_type, 4>;

    static_assert(D == 2 || D == 3, "Only 2D or 3D vectors are supported");

    constexpr vector(vector const&) noexcept = default;
    constexpr vector(vector&&) noexcept = default;
    constexpr vector& operator=(vector const&) noexcept = default;
    constexpr vector& operator=(vector&&) noexcept = default;

    /** Construct a vector from a lower dimension vector.
     */
    template<int E>
        requires(E < D)
    [[nodiscard]] constexpr vector(vector<value_type, E> const& other) noexcept : _v(static_cast<array_type>(other))
    {
        hi_axiom(holds_invariant());
    }

    /** Construct a vector from a higher dimension vector.
     * This will clear the values in the higher dimensions.
     */
    template<int E>
        requires(E > D)
    [[nodiscard]] constexpr explicit vector(vector<value_type, E> const& other) noexcept : _v(static_cast<array_type>(other))
    {
        for (std::size_t i = D; i != E; ++i) {
            _v[i] = value_type{0};
        }
        hi_axiom(holds_invariant());
    }

    /** Convert a vector to its array_type-nummeric_array.
     */
    [[nodiscard]] constexpr explicit operator array_type() const noexcept
    {
        return _v;
    }

    /** Construct a vector from a array_type-simd.
     */
    [[nodiscard]] constexpr explicit vector(array_type const& other) noexcept : _v(other)
    {
        hi_axiom(holds_invariant());
    }

    /** Construct a empty vector / zero length.
     */
    [[nodiscard]] constexpr vector() noexcept : _v(value_type{0}, value_type{0}, value_type{0}, value_type{0}) {}

    /** Construct a 2D vector from x and y elements.
     * @param x The x element.
     * @param y The y element.
     */
    [[nodiscard]] constexpr vector(value_type x, value_type y) noexcept
        requires(D == 2)
        : _v(x, y, value_type{0}, value_type{0})
    {
    }

    /** Construct a 3D vector from x, y and z elements.
     * @param x The x element.
     * @param y The y element.
     * @param z The z element.
     */
    [[nodiscard]] constexpr vector(value_type x, value_type y, value_type z = value_type{0}) noexcept
        requires(D == 3)
        : _v(x, y, z, value_type{0})
    {
    }

    /** Access the x element from the vector.
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr value_type& x() noexcept
    {
        return _v.x();
    }

    /** Access the y element from the vector.
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr value_type& y() noexcept
    {
        return _v.y();
    }

    /** Access the z element from the vector.
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr value_type& z() noexcept
        requires(D == 3)
    {
        return _v.z();
    }

    /** Access the x element from the vector.
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr value_type x() const noexcept
    {
        return _v.x();
    }

    /** Access the y element from the vector.
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr value_type y() const noexcept
    {
        return _v.y();
    }

    /** Access the z element from the vector.
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr value_type z() const noexcept
        requires(D == 3)
    {
        return _v.z();
    }

    /** Mirror this vector.
     * @return The mirrored vector.
     */
    [[nodiscard]] constexpr vector operator-() const noexcept
    {
        hi_axiom(holds_invariant());
        return vector{-_v};
    }

    template<int E>
        requires(E <= D)
    constexpr vector& operator+=(vector<value_type, E> const& rhs) noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        _v = _v + static_cast<array_type>(rhs);
        return *this;
    }

    /** Add two vectors from each other.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return A new vector.
     */
    [[nodiscard]] constexpr friend vector operator+(vector const& lhs, vector const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return vector{lhs._v + rhs._v};
    }

    /** Subtract two vectors from each other.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return A new vector.
     */
    [[nodiscard]] constexpr friend vector operator-(vector const& lhs, vector const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return vector{lhs._v - rhs._v};
    }

    /** Scale the vector by a scaler.
     * @param lhs The vector to scale.
     * @param rhs The scaling factor.
     * @return The scaled vector.
     */
    [[nodiscard]] constexpr friend vector operator*(vector const& lhs, value_type const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant());
        return vector{lhs._v * rhs};
    }

    /** Scale the vector by a scaler.
     * @param lhs The vector to scale.
     * @param rhs The scaling factor.
     * @return The scaled vector.
     */
    [[nodiscard]] constexpr friend vector operator*(value_type const& lhs, vector const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return vector{array_type::broadcast(lhs) * rhs._v};
    }

    /** Compare if two vectors are equal.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return True if both vectors are completely equal to each other.
     */
    [[nodiscard]] constexpr friend bool operator==(vector const& lhs, vector const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return equal(lhs._v, rhs._v);
    }

    /** Get the squared length of the vector.
     * @param rhs The vector.
     * @return The length of the vector.
     */
    [[nodiscard]] constexpr friend value_type squared_hypot(vector const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return squared_hypot<element_mask>(rhs._v);
    }

    /** Get the length of the vector.
     * @param rhs The vector.
     * @return The length of the vector.
     */
    [[nodiscard]] constexpr friend value_type hypot(vector const& rhs) noexcept
        requires std::is_floating_point_v<value_type>
    {
        hi_axiom(rhs.holds_invariant());
        return hypot<element_mask>(rhs._v);
    }

    /** Get the length of the vector.
     * @param rhs The vector.
     * @return One over the length of the vector.
     */
    [[nodiscard]] constexpr friend value_type rcp_hypot(vector const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return rcp_hypot<element_mask>(rhs._v);
    }

    /** Normalize a vector to a unit vector.
     * @param rhs The vector.
     * @return A vector with the same direction as the given vector, but its length is 1.0.
     */
    [[nodiscard]] constexpr friend vector normalize(vector const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return vector{normalize<element_mask>(rhs._v)};
    }

    /** Get the dot product between two vectors.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return The dot product from the two given vectors.
     */
    [[nodiscard]] constexpr friend value_type dot(vector const& lhs, vector const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return dot<element_mask>(lhs._v, rhs._v);
    }

    /** Get the determinate between two vectors.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return The dot product from the two given vectors.
     */
    [[nodiscard]] constexpr friend value_type det(vector const& lhs, vector const& rhs) noexcept
        requires(D == 2)
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return lhs.x() * rhs.y() - lhs.y() * rhs.x();
    }

    /** Mix the two vectors and get the lowest value of each element.
     * @param lhs The first vector.
     * @param rhs The first vector.
     * @return A vector that points the most left of both vectors, and most downward of both vectors.
     */
    template<int E>
    [[nodiscard]] friend constexpr auto min(vector const& lhs, vector<value_type, E> const& rhs) noexcept
    {
        return vector<value_type, std::max(D, E)>{min(static_cast<array_type>(lhs), static_cast<array_type>(rhs))};
    }

    /** Mix the two vectors and get the highest value of each element.
     * @param lhs The first vector.
     * @param rhs The first vector.
     * @return A vector that points the most right of both vectors, and most upward of both vectors.
     */
    template<int E>
    [[nodiscard]] friend constexpr auto max(vector const& lhs, vector<value_type, E> const& rhs) noexcept
    {
        return vector<value_type, std::max(D, E)>{max(static_cast<array_type>(lhs), static_cast<array_type>(rhs))};
    }

    /** Round the elements of the vector toward nearest integer.
     */
    [[nodiscard]] friend constexpr vector round(vector const& rhs) noexcept
    {
        return vector{round(static_cast<array_type>(rhs))};
    }

    /** Round the elements of the vector toward upward and to the right.
     */
    [[nodiscard]] friend constexpr vector ceil(vector const& rhs) noexcept
    {
        return vector{ceil(static_cast<array_type>(rhs))};
    }

    /** Round the elements of the vector toward downward and to the left.
     */
    [[nodiscard]] friend constexpr vector floor(vector const& rhs) noexcept
    {
        return vector{floor(static_cast<array_type>(rhs))};
    }

    /** Check if the vector is valid.
     * This function will check if w is zero, and with 2D vector is z is zero.
     */
    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() == value_type{0} && (D == 3 || _v.z() == value_type{0});
    }

    [[nodiscard]] friend std::string to_string(vector const& rhs) noexcept
    {
        if constexpr (D == 2) {
            return std::format("({}, {})", rhs._v.x(), rhs._v.y());
        } else if constexpr (D == 3) {
            return std::format("({}, {}, {})", rhs._v.x(), rhs._v.y(), rhs._v.z());
        } else {
            hi_static_no_default();
        }
    }

    friend std::ostream& operator<<(std::ostream& lhs, vector const& rhs) noexcept
    {
        return lhs << to_string(rhs);
    }

private:
    array_type _v;

    static constexpr std::size_t element_mask = (1_uz << D) - 1;
};

/** Get the cross product of one 2D vectors.
 * @param rhs The vector.
 * @return A vector perpendicular to the given vector.
 */
[[nodiscard]] constexpr vector<float, 2> cross(vector<float, 2> const& rhs) noexcept
{
    hi_axiom(rhs.holds_invariant());
    return vector<float, 2>{cross_2D(static_cast<f32x4>(rhs))};
}

/** Get the normal on a 2D vector.
 * @param rhs The vector.
 * @return A normal on the vector.
 */
[[nodiscard]] constexpr vector<float, 2> normal(vector<float, 2> const& rhs) noexcept
{
    hi_axiom(rhs.holds_invariant());
    return normalize(cross(rhs));
}

/** Get the normal on a 3D vector.
 * @param rhs The vector.
 * @param angle The angle around the vector, only value_type{0} is implemented (xy-plane)
 * @return A normal on the vector.
 */
[[nodiscard]] constexpr vector<float, 3> normal(vector<float, 3> const& rhs, float angle) noexcept
{
    if (angle != float{0}) {
        hi_not_implemented();
    }
    return normal(vector<float, 2>{f32x4{rhs}.xy00()});
}

/** Get the cross product between two 2D vectors.
 * This function is useful for finding the winding direction of the vectors,
 * when doing ray casting.
 *
 * @param lhs The first vector.
 * @param rhs The second vector.
 * @return A scaler representing the sharpness of the corner between the two vectors.
 */
[[nodiscard]] constexpr float cross(vector<float, 2> const& lhs, vector<float, 2> const& rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return cross_2D(static_cast<f32x4>(lhs), static_cast<f32x4>(rhs));
}

/** Get the cross product between two 3D vectors.
 * @param lhs The first vector.
 * @param rhs The second vector.
 * @return A vector that is perpendicular to the given vectors.
 */
[[nodiscard]] constexpr vector<float, 3> cross(vector<float, 3> const& lhs, vector<float, 3> const& rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return vector<float, 3>{cross_3D(static_cast<f32x4>(lhs), static_cast<f32x4>(rhs))};
}

} // namespace geo

using vector2 = geo::vector<float, 2>;
using vector3 = geo::vector<float, 3>;
using vector2i = geo::vector<int, 2>;
using vector3i = geo::vector<int, 3>;

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::geo::vector<float, 2>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::vector<float, 2> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "({}, {})", std::make_format_args(t.x(), t.y()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::vector<float, 3>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::vector<float, 3> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "({}, {}, {})", std::make_format_args(t.x(), t.y(), t.z()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::vector<int, 2>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::vector<int, 2> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "({}, {})", std::make_format_args(t.x(), t.y()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::vector<int, 3>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::vector<int, 3> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "({}, {}, {})", std::make_format_args(t.x(), t.y(), t.z()));
    }
};
