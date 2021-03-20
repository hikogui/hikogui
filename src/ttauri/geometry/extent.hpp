// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "vector.hpp"
#include "numeric_array.hpp"

namespace tt {
namespace geo {

/** A high-level geometric extent
 *
 * A extent, for both 2D or 3D is internally represented
 * as a 4D homogeneous extent. Which can be efficiently implemented
 * as a __m128 SSE register.
 */
template<int D>
class extent {
public:
    static_assert(D == 2 || D == 3, "Only 2D or 3D extents are supported");

    constexpr extent(extent const &) noexcept = default;
    constexpr extent(extent &&) noexcept = default;
    constexpr extent &operator=(extent const &) noexcept = default;
    constexpr extent &operator=(extent &&) noexcept = default;

    /** Construct a extent from a lower dimension extent.
     */
    template<int E>
    requires(E < D) [[nodiscard]] constexpr extent(extent<E> const &other) noexcept : _v(static_cast<f32x4>(other))
    {
        tt_axiom(is_valid());
    }

    /** Convert a extent to its f32x4-nummeric_array.
     */
    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return _v;
    }

    /** Construct a extent from a f32x4-numeric_array.
     */
    [[nodiscard]] constexpr explicit extent(f32x4 const &other) noexcept : _v(other)
    {
        tt_axiom(is_valid());
    }

    template<int E>
    [[nodiscard]] constexpr explicit operator vector<E>() const noexcept requires(E >= D)
    {
        tt_axiom(is_valid());
        return vector<E>{static_cast<f32x4>(*this)};
    }

    /** Construct a empty extent / zero length.
     */
    [[nodiscard]] constexpr extent() noexcept : _v(0.0f, 0.0f, 0.0f, 0.0f)
    {
        tt_axiom(is_valid());
    }

    /** Construct a 2D extent from x and y elements.
     * @param x The x element.
     * @param y The y element.
     */
    [[nodiscard]] constexpr extent(float width, float height) noexcept requires(D == 2) : _v(width, height, 0.0f, 0.0f)
    {
        tt_axiom(is_valid());
    }

    /** Construct a 3D extent from x, y and z elements.
     * @param x The x element.
     * @param y The y element.
     * @param z The z element.
     */
    [[nodiscard]] constexpr extent(float width, float height, float depth = 0.0f) noexcept requires(D == 3) :
        _v(width, height, depth, 0.0f)
    {
        tt_axiom(is_valid());
    }

    /** Access the x-as-width element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr float &width() noexcept
    {
        return _v.x();
    }

    /** Access the y-as-height element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr float &height() noexcept
    {
        return _v.y();
    }

    /** Access the z-as-depth element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr float &depth() noexcept requires(D == 3)
    {
        return _v.z();
    }

    /** Access the x-as-width element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr float const &width() const noexcept
    {
        return _v.x();
    }

    /** Access the y-as-height element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr float const &height() const noexcept
    {
        return _v.y();
    }

    /** Access the z-as-depth element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr float const &depth() const noexcept requires(D == 3)
    {
        return _v.z();
    }

    [[nodiscard]] constexpr vector<D> right() const noexcept
    {
        return vector<D>{_v.x000()};
    }

    [[nodiscard]] constexpr vector<D> up() const noexcept
    {
        return vector<D>{_v._0y00()};
    }

    /** Add two extents from each other.
     * @param lhs The first extent.
     * @param rhs The second extent.
     * @return A new extent.
     */
    [[nodiscard]] constexpr friend extent operator+(extent const &lhs, extent const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid());
        return extent{lhs._v + rhs._v};
    }

    /** Subtract two extents from each other.
     * @param lhs The first extent.
     * @param rhs The second extent.
     * @return A new extent.
     */
    [[nodiscard]] constexpr friend extent operator-(extent const &lhs, extent const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid());
        return extent{lhs._v - rhs._v};
    }

    /** Scale the extent by a scaler.
     * @param lhs The extent to scale.
     * @param rhs The scaling factor.
     * @return The scaled extent.
     */
    [[nodiscard]] constexpr friend extent operator*(extent const &lhs, float const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid());
        return extent{lhs._v * rhs};
    }

    template<int E>
    [[nodiscard]] constexpr friend auto operator+(extent const &lhs, vector<E> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid());
        tt_axiom(rhs.is_valid());

        return extent<std::max(D, E)>{static_cast<f32x4>(lhs) + static_cast<f32x4>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr friend auto operator+(vector<E> const &lhs, extent const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid());
        tt_axiom(rhs.is_valid());

        return vector<std::max(D, E)>{static_cast<f32x4>(lhs) + static_cast<f32x4>(rhs)};
    }

    /** Add a scaler to the extent.
     * @param lhs The extent to scale.
     * @param rhs The scaling factor.
     * @return The scaled extent.
     */
    [[nodiscard]] constexpr friend extent operator+(extent const &lhs, float const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid());

        auto r = extent{};
        for (size_t i = 0; i != D; ++i) {
            r._v[i] = lhs._v[i] + rhs;
        }

        return r;
    }

    /** Scale the extent by a scaler.
     * @param lhs The extent to scale.
     * @param rhs The scaling factor.
     * @return The scaled extent.
     */
    [[nodiscard]] constexpr friend extent operator*(float const &lhs, extent const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return extent{lhs * rhs._v};
    }

    /** Compare if two extents are equal.
     * @param lhs The first extent.
     * @param rhs The second extent.
     * @return True if both extents are completely equal to each other.
     */
    [[nodiscard]] constexpr friend bool operator==(extent const &lhs, extent const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid() && rhs.is_valid());
        return lhs._v == rhs._v;
    }

    /** Compare the size of the extents.
     */
    [[nodiscard]] constexpr friend bool operator<(extent const &lhs, extent const &rhs) noexcept requires (D == 2)
    {
        return lhs.width() < rhs.width() && lhs.height() < rhs.height();
    }

    /** Compare the size of the extents.
     */
    [[nodiscard]] constexpr friend bool operator<(extent const &lhs, extent const &rhs) noexcept requires(D == 3)
    {
        return lhs.width() < rhs.width() && lhs.height() < rhs.height() && lhs.depth() < rhs.depth();
    }

    /** Compare the size of the extents.
     */
    [[nodiscard]] constexpr friend bool operator<=(extent const &lhs, extent const &rhs) noexcept requires(D == 2)
    {
        return lhs.width() <= rhs.width() && lhs.height() <= rhs.height();
    }

    /** Compare the size of the extents.
     */
    [[nodiscard]] constexpr friend bool operator<=(extent const &lhs, extent const &rhs) noexcept requires(D == 3)
    {
        return lhs.width() <= rhs.width() && lhs.height() <= rhs.height() && lhs.depth() <= rhs.depth();
    }

    /** Compare the size of the extents.
     */
    [[nodiscard]] constexpr friend bool operator>(extent const &lhs, extent const &rhs) noexcept requires(D == 2)
    {
        return lhs.width() > rhs.width() && lhs.height() > rhs.height();
    }

    /** Compare the size of the extents.
     */
    [[nodiscard]] constexpr friend bool operator>(extent const &lhs, extent const &rhs) noexcept requires(D == 3)
    {
        return lhs.width() > rhs.width() && lhs.height() > rhs.height() && lhs.depth() > rhs.depth();
    }

    /** Compare the size of the extents.
     */
    [[nodiscard]] constexpr friend bool operator>=(extent const &lhs, extent const &rhs) noexcept requires(D == 2)
    {
        return lhs.width() >= rhs.width() && lhs.height() >= rhs.height();
    }

    /** Compare the size of the extents.
     */
    [[nodiscard]] constexpr friend bool operator>=(extent const &lhs, extent const &rhs) noexcept requires(D == 3)
    {
        return lhs.width() >= rhs.width() && lhs.height() >= rhs.height() && lhs.depth() >= rhs.depth();
    }

    /** Get the squared length of the extent.
     * @param rhs The extent.
     * @return The length of the extent.
     */
    [[nodiscard]] constexpr friend float squared_hypot(extent const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return squared_hypot<element_mask>(rhs._v);
    }

    /** Get the length of the extent.
     * @param rhs The extent.
     * @return The length of the extent.
     */
    [[nodiscard]] constexpr friend float hypot(extent const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return hypot<element_mask>(rhs._v);
    }

    /** Get the length of the extent.
     * @param rhs The extent.
     * @return One over the length of the extent.
     */
    [[nodiscard]] constexpr friend float rcp_hypot(extent const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return rcp_hypot<element_mask>(rhs._v);
    }

    /** Normalize a extent to a unit extent.
     * @param rhs The extent.
     * @return A extent with the same direction as the given extent, but its length is 1.0.
     */
    [[nodiscard]] constexpr friend extent normalize(extent const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return extent{normalize<element_mask>(rhs._v)};
    }

    [[nodiscard]] constexpr friend extent ceil(extent const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return extent{ceil(static_cast<f32x4>(rhs))};
    }

    [[nodiscard]] constexpr friend extent floor(extent const &rhs) noexcept
    {
        tt_axiom(rhs.is_valid());
        return extent{floor(static_cast<f32x4>(rhs))};
    }

    /** Check if the extent is valid.
     * Extends must be positive.
     * This function will check if w is zero, and with 2D extent is z is zero.
     */
    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return _v.x() >= 0.0f && _v.y() >= 0.0f && _v.z() >= 0.0f && _v.w() == 0.0f && (D == 3 || _v.z() == 0.0f);
    }

    [[nodiscard]] friend std::string to_string(extent const &rhs) noexcept
    {
        if constexpr (D == 2) {
            return fmt::format("[{}, {}]", rhs._v.x(), rhs._v.y());
        } else if constexpr (D == 3) {
            return fmt::format("[{}, {}, {}]", rhs._v.x(), rhs._v.y(), rhs._v.z());
        } else {
            tt_static_no_default();
        }
    }

    friend std::ostream &operator<<(std::ostream &lhs, extent const &rhs) noexcept
    {
        return lhs << to_string(rhs);
    }

private:
    f32x4 _v;

    static constexpr size_t element_mask = (1_uz << D) - 1;
};

}

using extent2 = geo::extent<2>;
using extent3 = geo::extent<3>;

} // namespace tt
