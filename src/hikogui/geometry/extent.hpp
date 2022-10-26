// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "vector.hpp"
#include "../rapid/numeric_array.hpp"
#include <compare>

namespace hi::inline v1 {
namespace geo {
template<int D>
class scale;

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
        hi_axiom(holds_invariant());
    }

    /** Convert a extent to its f32x4-nummeric_array.
     */
    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr explicit extent(f32x4 const &other) noexcept : _v(other) {}

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        if constexpr (D == 2) {
            return _v.x() != 0.0f or _v.y() != 0.0f;
        } else if constexpr (D == 3) {
            return _v.x() != 0.0f or _v.y() != 0.0f or _v.z() != 0.0f;
        } else {
            hi_no_default();
        }
    }

    template<int E>
    [[nodiscard]] constexpr explicit operator vector<E>() const noexcept requires(E >= D)
    {
        hi_axiom(holds_invariant());
        return vector<E>{static_cast<f32x4>(*this)};
    }

    /** Construct a empty extent / zero length.
     */
    [[nodiscard]] constexpr extent() noexcept : _v(0.0f, 0.0f, 0.0f, 0.0f)
    {
        hi_axiom(holds_invariant());
    }

    /** Construct a 2D extent from the width and height.
     * @param width The width element.
     * @param height The height element.
     */
    [[nodiscard]] constexpr extent(float width, float height) noexcept requires(D == 2) : _v(width, height, 0.0f, 0.0f)
    {
        hi_axiom(holds_invariant());
    }

    /** Construct a 3D extent from width, height and depth.
     * @param width The width element.
     * @param height The height element.
     * @param depth The depth element.
     */
    [[nodiscard]] constexpr extent(float width, float height, float depth = 0.0f) noexcept requires(D == 3) :
        _v(width, height, depth, 0.0f)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] static constexpr extent infinity() noexcept requires(D == 2)
    {
        return extent{std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
    }

    [[nodiscard]] static constexpr extent infinity() noexcept requires(D == 3)
    {
        return extent{
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity(),
            std::numeric_limits<float>::infinity()};
    }

    [[nodiscard]] static constexpr extent large() noexcept requires(D == 2)
    {
        return extent{32767.0f, 32767.0f};
    }

    [[nodiscard]] static constexpr extent large() noexcept requires(D == 3)
    {
        return extent{32767.0f, 32767.0f, 32767.0f};
    }

    [[nodiscard]] static constexpr extent nan() noexcept requires(D == 2)
    {
        auto r = extent{};
        r._v.x() = std::numeric_limits<float>::signaling_NaN();
        r._v.y() = std::numeric_limits<float>::signaling_NaN();
        return r;
    }

    [[nodiscard]] static constexpr extent nan() noexcept requires(D == 3)
    {
        auto r = extent{};
        r._v.x() = std::numeric_limits<float>::signaling_NaN();
        r._v.y() = std::numeric_limits<float>::signaling_NaN();
        r._v.z() = std::numeric_limits<float>::signaling_NaN();
        return r;
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

    constexpr extent &operator+=(extent const &rhs) noexcept
    {
        return *this = *this + rhs;
    }

    /** Add two extents from each other.
     * @param lhs The first extent.
     * @param rhs The second extent.
     * @return A new extent.
     */
    [[nodiscard]] constexpr friend extent operator+(extent const &lhs, extent const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return extent{lhs._v + rhs._v};
    }

    /** Subtract two extents from each other.
     * @param lhs The first extent.
     * @param rhs The second extent.
     * @return A new extent.
     */
    [[nodiscard]] constexpr friend extent operator-(extent const &lhs, extent const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return extent{lhs._v - rhs._v};
    }

    constexpr friend scale<D> operator/(extent const &lhs, extent const &rhs) noexcept;

    /** Scale the extent by a scaler.
     * @param lhs The extent to scale.
     * @param rhs The scaling factor.
     * @return The scaled extent.
     */
    [[nodiscard]] constexpr friend extent operator*(extent const &lhs, float const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant());
        return extent{lhs._v * rhs};
    }

    template<int E>
    [[nodiscard]] constexpr friend auto operator+(extent const &lhs, vector<E> const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant());
        hi_axiom(rhs.holds_invariant());

        return extent<std::max(D, E)>{static_cast<f32x4>(lhs) + static_cast<f32x4>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr friend auto operator+(vector<E> const &lhs, extent const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant());
        hi_axiom(rhs.holds_invariant());

        return vector<std::max(D, E)>{static_cast<f32x4>(lhs) + static_cast<f32x4>(rhs)};
    }

    /** Add a scaler to the extent.
     * @param lhs The extent to scale.
     * @param rhs The scaling factor.
     * @return The scaled extent.
     */
    [[nodiscard]] constexpr friend extent operator+(extent const &lhs, float const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant());

        auto r = extent{};
        for (std::size_t i = 0; i != D; ++i) {
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
        hi_axiom(rhs.holds_invariant());
        return extent{lhs * rhs._v};
    }

    /** Compare if two extents are equal.
     * @param lhs The first extent.
     * @param rhs The second extent.
     * @return True if both extents are completely equal to each other.
     */
    [[nodiscard]] constexpr friend bool operator==(extent const &lhs, extent const &rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return lhs._v == rhs._v;
    }

    [[nodiscard]] constexpr friend std::partial_ordering operator<=>(extent const &lhs, extent const &rhs) noexcept
        requires(D == 3)
    {
        constexpr std::size_t mask = 0b111;

        hilet equal = eq(lhs._v, rhs._v) & mask;
        if (equal == mask) {
            // Only equivalent if all elements are equal.
            return std::partial_ordering::equivalent;
        }

        hilet less = lt(lhs._v, rhs._v) & mask;
        if ((less | equal) == mask) {
            // If one or more elements is less (but none are greater) then the ordering is less.
            return std::partial_ordering::less;
        }

        hilet greater = lt(lhs._v, rhs._v) & mask;
        if ((greater | equal) == mask) {
            // If one or more elements is greater (but none are less) then the ordering is greater.
            return std::partial_ordering::greater;
        }

        // Some elements are less and others are greater, we don't have an ordering.
        return std::partial_ordering::unordered;
    }

    [[nodiscard]] constexpr friend std::partial_ordering operator<=>(extent const &lhs, extent const &rhs) noexcept
        requires(D == 2)
    {
        constexpr std::size_t mask = 0b11;

        hilet equal = eq(lhs._v, rhs._v) & mask;
        if (equal == mask) {
            // Only equivalent if all elements are equal.
            return std::partial_ordering::equivalent;
        }

        hilet less = lt(lhs._v, rhs._v) & mask;
        if ((less | equal) == mask) {
            // If one or more elements is less (but none are greater) then the ordering is less.
            return std::partial_ordering::less;
        }

        hilet greater = lt(lhs._v, rhs._v) & mask;
        if ((greater | equal) == mask) {
            // If one or more elements is greater (but none are less) then the ordering is greater.
            return std::partial_ordering::greater;
        }

        // Some elements are less and others are greater, we don't have an ordering.
        return std::partial_ordering::unordered;
    }

    /** Get the squared length of the extent.
     * @param rhs The extent.
     * @return The length of the extent.
     */
    [[nodiscard]] hi_force_inline constexpr friend float squared_hypot(extent const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return squared_hypot<element_mask>(rhs._v);
    }

    /** Get the length of the extent.
     * @param rhs The extent.
     * @return The length of the extent.
     */
    [[nodiscard]] constexpr friend float hypot(extent const &rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return hypot<element_mask>(rhs._v);
    }

    /** Get the length of the extent.
     * @param rhs The extent.
     * @return One over the length of the extent.
     */
    [[nodiscard]] constexpr friend float rcp_hypot(extent const &rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return rcp_hypot<element_mask>(rhs._v);
    }

    /** Normalize a extent to a unit extent.
     * @param rhs The extent.
     * @return A extent with the same direction as the given extent, but its length is 1.0.
     */
    [[nodiscard]] constexpr friend extent normalize(extent const &rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return extent{normalize<element_mask>(rhs._v)};
    }

    [[nodiscard]] constexpr friend extent ceil(extent const &rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return extent{ceil(f32x4{rhs})};
    }

    [[nodiscard]] constexpr friend extent floor(extent const &rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return extent{floor(static_cast<f32x4>(rhs))};
    }

    [[nodiscard]] constexpr friend extent round(extent const &rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return extent{round(static_cast<f32x4>(rhs))};
    }

    [[nodiscard]] constexpr friend extent min(extent const &lhs, extent const &rhs) noexcept
    {
        return extent{min(static_cast<f32x4>(lhs), static_cast<f32x4>(rhs))};
    }

    [[nodiscard]] constexpr friend extent max(extent const &lhs, extent const &rhs) noexcept
    {
        return extent{max(static_cast<f32x4>(lhs), static_cast<f32x4>(rhs))};
    }

    [[nodiscard]] constexpr friend extent clamp(extent const &value, extent const &min, extent const &max) noexcept
    {
        return extent{clamp(static_cast<f32x4>(value), static_cast<f32x4>(min), static_cast<f32x4>(max))};
    }

    /** Check if the extent is valid.
     * Extends must be positive.
     * This function will check if w is zero, and with 2D extent is z is zero.
     */
    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.x() >= 0.0f && _v.y() >= 0.0f && _v.z() >= 0.0f && _v.w() == 0.0f && (D == 3 || _v.z() == 0.0f);
    }

    [[nodiscard]] friend std::string to_string(extent const &rhs) noexcept
    {
        if constexpr (D == 2) {
            return std::format("[{}, {}]", rhs._v.x(), rhs._v.y());
        } else if constexpr (D == 3) {
            return std::format("[{}, {}, {}]", rhs._v.x(), rhs._v.y(), rhs._v.z());
        } else {
            hi_static_no_default();
        }
    }

    friend std::ostream &operator<<(std::ostream &lhs, extent const &rhs) noexcept
    {
        return lhs << to_string(rhs);
    }

private:
    f32x4 _v;

    static constexpr std::size_t element_mask = (1_uz << D) - 1;
};

} // namespace geo

using extent2 = geo::extent<2>;
using extent3 = geo::extent<3>;

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::geo::extent<2>, CharT> {
    auto parse(auto &pc)
    {
        return pc.end();
    }

    auto format(hi::geo::extent<2> const &t, auto &fc)
    {
        return std::vformat_to(fc.out(), "[{}, {}]", std::make_format_args(t.width(), t.height()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::extent<3>, CharT> : formatter<float, CharT> {
    auto parse(auto &pc)
    {
        return pc.end();
    }

    auto format(hi::geo::extent<3> const &t, auto &fc)
    {
        return std::vformat_to(fc.out(), "[{}, {}, {}]", std::make_format_args(t.width(), t.height(), t.depth()));
    }
};
