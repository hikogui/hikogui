// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../numeric_array.hpp"

namespace tt {
namespace geo {

/** A high-level geometric vector
 * Part of the high-level vector, point, mat and color types.
 *
 * A vector, for both 2D or 3D is internally represented
 * as a 4D homogeneous vector. Which can be efficiently implemented
 * as a __m128 SSE register.
 */
template<int D>
class vector {
public:
    static_assert(D == 2 || D == 3, "Only 2D or 3D vectors are supported");

    constexpr vector(vector const &) noexcept = default;
    constexpr vector(vector &&) noexcept = default;
    constexpr vector &operator=(vector const &) noexcept = default;
    constexpr vector &operator=(vector &&) noexcept = default;

    /** Construct a vector from a lower dimension vector.
     */
    template<int E>
    requires(E < D) [[nodiscard]] constexpr vector(vector<E> const &other) noexcept : _v(static_cast<f32x4>(other))
    {
        tt_axiom(is_valid());
    }

    /** Convert a vector to its f32x4-nummeric_array.
     */
    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return _v;
    }

    /** Construct a vector from a f32x4-numeric_array.
     */
    [[nodiscard]] constexpr explicit vector(f32x4 const &other) noexcept : _v(other)
    {
        tt_axiom(is_valid());
    }

    /** Construct a empty vector / zero length.
     */
    [[nodiscard]] constexpr vector() noexcept : _v(0.0f, 0.0f, 0.0f, 0.0f) {}

    /** Construct a 2D vector from x and y elements.
     * @param x The x element.
     * @param y The y element.
     */
    [[nodiscard]] constexpr vector(float x, float y) noexcept requires(D == 2) : _v(x, y, 0.0f, 0.0f) {}

    /** Construct a 3D vector from x, y and z elements.
     * @param x The x element.
     * @param y The y element.
     * @param z The z element.
     */
    [[nodiscard]] constexpr vector(float x, float y, float z = 0.0f) noexcept requires(D == 3) : _v(x, y, z, 0.0f) {}

    /** Access the x element from the vector.
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr float &x() noexcept
    {
        return _v.x();
    }

    /** Access the y element from the vector.
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr float &y() noexcept
    {
        return _v.y();
    }

    /** Access the z element from the vector.
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr float &z() noexcept requires(D == 3)
    {
        return _v.z();
    }

    /** Access the x element from the vector.
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr float const &x() const noexcept
    {
        return _v.x();
    }

    /** Access the y element from the vector.
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr float const &y() const noexcept
    {
        return _v.y();
    }

    /** Access the z element from the vector.
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr float const &z() const noexcept requires(D == 3)
    {
        return _v.z();
    }

    /** Access the x-as-width element from the vector.
     * A vector can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr float &width() noexcept
    {
        return _v.x();
    }

    /** Access the y-as-height element from the vector.
     * A vector can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr float &height() noexcept
    {
        return _v.y();
    }

    /** Access the z-as-depth element from the vector.
     * A vector can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr float &depth() noexcept requires(D == 3)
    {
        return _v.z();
    }

    /** Access the x-as-width element from the vector.
     * A vector can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr float const &width() const noexcept
    {
        return _v.x();
    }

    /** Access the y-as-height element from the vector.
     * A vector can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr float const &height() const noexcept
    {
        return _v.y();
    }

    /** Access the z-as-depth element from the vector.
     * A vector can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr float const &depth() const noexcept requires(D == 3)
    {
        return _v.z();
    }

    /** Mirror this vector.
     * @return The mirrored vector.
     */
    [[nodiscard]] constexpr vector operator-() const noexcept
    {
        tt_axiom(is_valid());
        return vector{-_v};
    }

    /** Add two vectors from each other.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return A new vector.
     */
    [[nodiscard]] constexpr friend vector operator+(vector const &lhs, vector const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid());
        return vector{lhs._v + rhs._v};
    }

    /** Subtract two vectors from each other.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return A new vector.
     */
    [[nodiscard]] constexpr friend vector operator-(vector const &lhs, vector const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid());
        return vector{lhs._v - rhs._v};
    }

    /** Scale the vector by a scaler.
     * @param lhs The vector to scale.
     * @param rhs The scaling factor.
     * @return The scaled vector.
     */
    [[nodiscard]] constexpr friend vector operator*(vector const &lhs, float const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid());
        return vector{lhs._v * rhs};
    }

    /** Compare if two vectors are equal.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return True if both vectors are completely equal to each other.
     */
    [[nodiscard]] constexpr friend bool operator==(vector const &lhs, vector const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid());
        return lhs._v == rhs._v;
    }

    /** Get the length of the vector.
     * @param rhs The vector.
     * @return The length of the vector.
     */
    [[nodiscard]] constexpr friend float hypot(vector const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return hypot<element_mask>(rhs._v);
    }

    /** Get the length of the vector.
     * @param rhs The vector.
     * @return One over the length of the vector.
     */
    [[nodiscard]] constexpr friend float rcp_hypot(vector const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return rcp_hypot<element_mask>(rhs._v);
    }

    /** Normalize a vector to a unit vector.
     * @param rhs The vector.
     * @return A vector with the same direction as the given vector, but its length is 1.0.
     */
    [[nodiscard]] constexpr friend vector normalize(vector const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return vector{normalize<element_mask>(rhs._v)};
    }

    /** Get the dot product between two vectors.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return The dot product from the two given vectors.
     */
    [[nodiscard]] constexpr friend float dot(vector const &lhs, vector const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid());
        return dot<element_mask>(lhs._v, rhs._v);
    }

    /** Check if the vector is valid.
     * This function will check if w is zero, and with 2D vector is z is zero.
     */
    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return _v.w() == 0.0f && (D == 3 || _v.z() == 0.0f);
    }

private:
    f32x4 _v;

    static constexpr size_t element_mask = (1_uz << D) - 1;
};

/** Get the cross product of one 2D vectors.
 * @param rhs The vector.
 * @return A vector perpendicular to the given vector.
 */
[[nodiscard]] constexpr vector<2> cross(vector<2> const &rhs) noexcept
{
    tt_axiom(rhs.is_valid());
    return vector<2>{cross_2D(static_cast<f32x4>(rhs))};
}

/** Get the cross product between two 2D vectors.
 * This function is useful for finding the winding direction of the vectors,
 * when doing ray casting.
 *
 * @param lhs The first vector.
 * @param rhs The second vector.
 * @return A scaler representing the sharpness of the corner between the two vectors.
 */
[[nodiscard]] constexpr float cross(vector<2> const &lhs, vector<2> const &rhs) noexcept
{
    tt_axiom(lhs.is_valid() && rhs.is_valid());
    return cross_2D(static_cast<f32x4>(lhs), static_cast<f32x4>(rhs));
}

/** Get the cross product between two 3D vectors.
 * @param lhs The first vector.
 * @param rhs The second vector.
 * @return A vector that is perpendicular to the given vectors.
 */
[[nodiscard]] constexpr vector<3> cross(vector<3> const &lhs, vector<3> const &rhs) noexcept
{
    tt_axiom(lhs.is_valid() && rhs.is_valid());
    return vector<3>{cross_3D(static_cast<f32x4>(lhs), static_cast<f32x4>(rhs))};
}

}

using vector2 = geo::vector<2>;
using vector3 = geo::vector<3>;

} // namespace tt
