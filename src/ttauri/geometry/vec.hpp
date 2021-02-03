

#pragma once

#include "numeric_array.hpp"

namespace tt {

/** A geometric vector
 * Part of the vec, point, matrix and color types.
 *
 * A vector, for both 2D or 3D is internally represented
 * as a 4D homogeneous vector. Which can be efficiently implemented
 * as a __m128 SSE register.
 */
template<int D> requires (D == 2 || D == 3)
class vec {
public:
    constexpr vec(vec const &) noexcept = default;
    constexpr vec(vec &&) noexcept = default;
    constexpr vec &operator=(vec const &) noexcept = default;
    constexpr vec &operator=(vec &&) noexcept = default;

    /** Construct a vector from a lower dimension vector.
     */
    template<int E> requires(E < D)
    [[nodiscard]] constexpr vec(vec<E> other) noexcept :
        _v(other._v)
    {
        tt_axiom(is_valid());
    }

    /** Convert a vector to its f32x4-nummeric_array.
     */
    [[nodiscard]] constexpr explicit operator f32x4 () const noexcept
    {
        return _v;
    }

    /** Construct a vector from a f32x4-numeric_array.
     */
    [[nodiscard]] constexpr explicit vec(f32x4 const &other) noexcept : _v(other)
    {
        tt_axiom(is_valid());
    }

    /** Construct a empty vector / zero length.
     */
    [[nodiscard]] constexpr vec() noexcept :
        _v(0.0f, 0.0f, 0.0f, 0.0f) {}

    /** Construct a 2D vector from x and y elements.
     * @param x The x element.
     * @param y The y element.
     */
    [[nodiscard]] constexpr vec(float x, float y) noexcept requires (D == 2) :
        _v(x, y, 0.0f, 0.0f) {}

    /** Construct a 3D vector from x and y elements.
     * @param x The x element.
     * @param y The y element.
     * @param z The z element.
     */
    [[nodiscard]] constexpr vec(float x, float y, float z=0.0f) noexcept requires (D == 3) :
        _v(x, y, z, 0.0f) {}

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
    [[nodiscard]] constexpr float &z() noexcept requires (D == 3)
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
    [[nodiscard]] constexpr float const &z() const noexcept requires (D == 3)
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
    [[nodiscard]] constexpr float &depth() noexcept requires (D == 3)
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
    [[nodiscard]] constexpr float const &depth() const noexcept requires (D == 3)
    {
        return _v.z();
    }

    /** Mirror this vector.
     * @return The mirrored vector.
     */
    [[nodiscard]] constexpr friend vec operator-() noexcept
    {
        tt_axiom(is_valid());
        return { -_v };
    }

    /** Add two vectors from each other.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return A new vector.
     */
    [[nodiscard]] constexpr friend vec operator+(vec const &lhs, vec const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid);
        return vec{lhs._v + rhs._v};
    }

    /** Subtract two vectors from each other.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return A new vector.
     */
    [[nodiscard]] constexpr friend vec operator-(vec const &lhs, vec const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid);
        return vec{lhs._v - rhs._v};
    }

    /** Scale the vector by a scaler.
     * @param lhs The vector to scale.
     * @param rhs The scaling factor.
     * @return The scaled vector.
     */
    [[nodiscard]] constexpr friend vec operator*(vec const &lhs, float const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid());
        return vec{lhs._v * rhs};
    }

    /** Get the length of the vector.
     * @param rhs The vector.
     * @return The length of the vector.
     */
    [[nodiscard]] constexpr friend float hypot(vec const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return hypot<element_mask>(rhs._v);
    }

    /** Get the length of the vector.
     * @param rhs The vector.
     * @return One over the length of the vector.
     */
    [[nodiscard]] constexpr friend float rcp_hypot(vec const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return rcp_hypot<element_mask>(rhs._v);
    }

    /** Normalize a vector to a unit vector.
     * @param rhs The vector.
     * @return A vector with the same direction as the given vector, but its length is 1.0.
     */
    [[nodiscard]] constexpr friend vec normalize(vec const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return normalize<element_mask>(rhs._v};
    }

    /** Get the dot product between two vectors.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return The dot product from the two given vectors.
     */
    [[nodiscard]] constexpr friend float dot(vec const &lhs, vec const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid());
        return dot<element_mask>{lhs._v, rhs._v};
    }

    /** Get the cross product between two 2D vectors.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return A scaler representing the sharpness of the corner between the two vectors.
     */
    [[nodiscard]] constexpr friend float cross(vec const &lhs, vec const &rhs) noexcept requires (D == 2)
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid());
        return viktor_cross<2>{lhs._v, rhs._v};
    }

    /** Get the cross product between two 3D vectors.
     * @param lhs The first vector.
     * @param rhs The second vector.
     * @return A vector that is perpedicular to the given vector.
     */
    [[nodiscard]] constexpr friend vec cross(vec const &lhs, vec const &rhs) noexcept requires (D == 3)
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid());
        return vec{cross<3>{lhs._v, rhs._v}};
    }

private:
    f32x4 _v;

    constexpr auto element_mask = (1_uz << D) - 1;

    /** Check if the vector is valid.
     */
    [[nodiscard]] bool is_valid() const noexcept {
        return _v.w() == 0.0f && (D == 3 || _v.z() == 0.0f);
    }

    template<int D>
    friend class point;
};

}

