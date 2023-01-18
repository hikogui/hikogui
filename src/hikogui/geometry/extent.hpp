// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file geometry/extent.hpp Defined the geo::extent, extent2 and extent3 types.
 * @ingroup geometry
 */

#pragma once

#include "vector.hpp"
#include "../SIMD/module.hpp"
#include "../utility/module.hpp"
#include <compare>

namespace hi { inline namespace v1 {
namespace geo {

template<int D>
class scale;

/** A high-level geometric extent
 * @ingroup geometry
 *
 * A extent, for both 2D or 3D is internally represented
 * as a 4D homogeneous extent. Which can be efficiently implemented
 * as a __m128 SSE register.
 */
template<typename T, int D>
class extent {
public:
    using value_type = T;
    using array_type = simd<value_type, 4>;

    static_assert(D == 2 || D == 3, "Only 2D or 3D extents are supported");

    constexpr extent(extent const&) noexcept = default;
    constexpr extent(extent&&) noexcept = default;
    constexpr extent& operator=(extent const&) noexcept = default;
    constexpr extent& operator=(extent&&) noexcept = default;

    [[nodiscard]] constexpr static extent large() noexcept
    {
        return {large_number_v<value_type>, large_number_v<value_type>};
    }

    /** Construct a extent from a lower dimension extent.
     */
    template<int E>
        requires(E < D)
    [[nodiscard]] constexpr extent(extent<value_type, E> const& other) noexcept : _v(static_cast<array_type>(other))
    {
        hi_axiom(holds_invariant());
    }

    /** Convert a extent to its array_type-nummeric_array.
     */
    [[nodiscard]] constexpr explicit operator array_type() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr explicit extent(array_type const& other) noexcept : _v(other) {}

    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        if constexpr (D == 2) {
            return _v.x() != value_type{0} or _v.y() != value_type{0};
        } else if constexpr (D == 3) {
            return _v.x() != value_type{0} or _v.y() != value_type{0} or _v.z() != value_type{0};
        } else {
            hi_no_default();
        }
    }

    template<int E>
    [[nodiscard]] constexpr explicit operator vector<value_type, E>() const noexcept
        requires(E >= D)
    {
        hi_axiom(holds_invariant());
        return vector<value_type, E>{static_cast<array_type>(*this)};
    }

    /** Construct a empty extent / zero length.
     */
    [[nodiscard]] constexpr extent() noexcept : _v(value_type{0}, value_type{0}, value_type{0}, value_type{0})
    {
        hi_axiom(holds_invariant());
    }

    /** Construct a 2D extent from the width and height.
     * @param width The width element.
     * @param height The height element.
     */
    [[nodiscard]] constexpr extent(value_type width, value_type height) noexcept
        requires(D == 2)
        : _v(width, height, value_type{0}, value_type{0})
    {
        hi_axiom(holds_invariant());
    }

    /** Construct a 3D extent from width, height and depth.
     * @param width The width element.
     * @param height The height element.
     * @param depth The depth element.
     */
    [[nodiscard]] constexpr extent(value_type width, value_type height, value_type depth = value_type{0}) noexcept
        requires(D == 3)
        : _v(width, height, depth, value_type{0})
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] static constexpr extent infinity() noexcept
        requires(D == 2)
    {
        return extent{std::numeric_limits<value_type>::infinity(), std::numeric_limits<value_type>::infinity()};
    }

    [[nodiscard]] static constexpr extent infinity() noexcept
        requires(D == 3)
    {
        return extent{
            std::numeric_limits<value_type>::infinity(),
            std::numeric_limits<value_type>::infinity(),
            std::numeric_limits<value_type>::infinity()};
    }

    [[nodiscard]] static constexpr extent large() noexcept
        requires(D == 2)
    {
        return extent{value_type{16777216}, value_type{16777216}};
    }

    [[nodiscard]] static constexpr extent large() noexcept
        requires(D == 3)
    {
        return extent{value_type{16777216}, value_type{16777216}, value_type{16777216}};
    }

    [[nodiscard]] static constexpr extent nan() noexcept
        requires std::is_same_v<value_type, float> and (D == 2)
    {
        auto r = extent{};
        r._v.x() = std::numeric_limits<value_type>::signaling_NaN();
        r._v.y() = std::numeric_limits<value_type>::signaling_NaN();
        return r;
    }

    [[nodiscard]] static constexpr extent nan() noexcept
        requires std::is_same_v<value_type, float> and (D == 3)
    {
        auto r = extent{};
        r._v.x() = std::numeric_limits<value_type>::signaling_NaN();
        r._v.y() = std::numeric_limits<value_type>::signaling_NaN();
        r._v.z() = std::numeric_limits<value_type>::signaling_NaN();
        return r;
    }

    /** Access the x-as-width element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr value_type& width() noexcept
    {
        return _v.x();
    }

    /** Access the y-as-height element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr value_type& height() noexcept
    {
        return _v.y();
    }

    /** Access the z-as-depth element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr value_type& depth() noexcept
        requires(D == 3)
    {
        return _v.z();
    }

    /** Access the x-as-width element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the x element.
     */
    [[nodiscard]] constexpr value_type width() const noexcept
    {
        return _v.x();
    }

    /** Access the y-as-height element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the y element.
     */
    [[nodiscard]] constexpr value_type height() const noexcept
    {
        return _v.y();
    }

    /** Access the z-as-depth element from the extent.
     * A extent can be seen as having a width, height and depth,
     * these accessors are aliases for x, y, and z.
     *
     * @return a reference to the z element.
     */
    [[nodiscard]] constexpr value_type depth() const noexcept
        requires(D == 3)
    {
        return _v.z();
    }

    [[nodiscard]] constexpr vector<value_type, D> right() const noexcept
    {
        return vector<value_type, D>{_v.x000()};
    }

    [[nodiscard]] constexpr vector<value_type, D> up() const noexcept
    {
        return vector<value_type, D>{_v._0y00()};
    }

    constexpr extent& operator+=(extent const& rhs) noexcept
    {
        return *this = *this + rhs;
    }

    /** Add two extents from each other.
     * @param lhs The first extent.
     * @param rhs The second extent.
     * @return A new extent.
     */
    [[nodiscard]] constexpr friend extent operator+(extent const& lhs, extent const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return extent{lhs._v + rhs._v};
    }

    /** Subtract two extents from each other.
     * @param lhs The first extent.
     * @param rhs The second extent.
     * @return A new extent.
     */
    [[nodiscard]] constexpr friend extent operator-(extent const& lhs, extent const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return extent{lhs._v - rhs._v};
    }

    constexpr friend scale<D> operator/(extent const& lhs, extent const& rhs) noexcept;

    /** Scale the extent by a scaler.
     * @param lhs The extent to scale.
     * @param rhs The scaling factor.
     * @return The scaled extent.
     */
    [[nodiscard]] constexpr friend extent operator*(extent const& lhs, value_type const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant());
        return extent{lhs._v * rhs};
    }

    template<int E>
    [[nodiscard]] constexpr friend auto operator+(extent const& lhs, vector<value_type, E> const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant());
        hi_axiom(rhs.holds_invariant());

        return extent<value_type, std::max(D, E)>{static_cast<array_type>(lhs) + static_cast<array_type>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr friend auto operator+(vector<value_type, E> const& lhs, extent const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant());
        hi_axiom(rhs.holds_invariant());

        return vector<value_type, std::max(D, E)>{static_cast<array_type>(lhs) + static_cast<array_type>(rhs)};
    }

    /** Add a scaler to the extent.
     * @param lhs The extent to scale.
     * @param rhs The scaling factor.
     * @return The scaled extent.
     */
    [[nodiscard]] constexpr friend extent operator+(extent const& lhs, value_type const& rhs) noexcept
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
    [[nodiscard]] constexpr friend extent operator*(value_type const& lhs, extent const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return extent{lhs * rhs._v};
    }

    /** Compare if two extents are equal.
     * @param lhs The first extent.
     * @param rhs The second extent.
     * @return True if both extents are completely equal to each other.
     */
    [[nodiscard]] constexpr friend bool operator==(extent const& lhs, extent const& rhs) noexcept
    {
        hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
        return equal(lhs._v, rhs._v);
    }

    [[nodiscard]] constexpr friend std::partial_ordering operator<=>(extent const& lhs, extent const& rhs) noexcept
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

    [[nodiscard]] constexpr friend std::partial_ordering operator<=>(extent const& lhs, extent const& rhs) noexcept
        requires(D == 2)
    {
        constexpr std::size_t mask = 0b11;

        hilet equal = (lhs._v == rhs._v).mask() & mask;
        if (equal == mask) {
            // Only equivalent if all elements are equal.
            return std::partial_ordering::equivalent;
        }

        hilet less = (lhs._v < rhs._v).mask() & mask;
        if ((less | equal) == mask) {
            // If one or more elements is less (but none are greater) then the ordering is less.
            return std::partial_ordering::less;
        }

        hilet greater = (lhs._v > rhs._v).mask() & mask;
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
    [[nodiscard]] hi_force_inline constexpr friend value_type squared_hypot(extent const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return squared_hypot<element_mask>(rhs._v);
    }

    /** Get the length of the extent.
     * @param rhs The extent.
     * @return The length of the extent.
     */
    [[nodiscard]] constexpr friend value_type hypot(extent const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return hypot<element_mask>(rhs._v);
    }

    /** Get the length of the extent.
     * @param rhs The extent.
     * @return One over the length of the extent.
     */
    [[nodiscard]] constexpr friend value_type rcp_hypot(extent const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return rcp_hypot<element_mask>(rhs._v);
    }

    /** Normalize a extent to a unit extent.
     * @param rhs The extent.
     * @return A extent with the same direction as the given extent, but its length is 1.0.
     */
    [[nodiscard]] constexpr friend extent normalize(extent const& rhs) noexcept
    {
        hi_axiom(rhs.holds_invariant());
        return extent{normalize<element_mask>(rhs._v)};
    }

    [[nodiscard]] constexpr friend extent ceil(extent const& rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        hi_axiom(rhs.holds_invariant());
        return extent{ceil(array_type{rhs})};
    }

    [[nodiscard]] constexpr friend extent floor(extent const& rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        hi_axiom(rhs.holds_invariant());
        return extent{floor(static_cast<array_type>(rhs))};
    }

    [[nodiscard]] constexpr friend extent round(extent const& rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        hi_axiom(rhs.holds_invariant());
        return extent{round(static_cast<array_type>(rhs))};
    }

    [[nodiscard]] constexpr friend extent min(extent const& lhs, extent const& rhs) noexcept
    {
        return extent{min(static_cast<array_type>(lhs), static_cast<array_type>(rhs))};
    }

    [[nodiscard]] constexpr friend extent max(extent const& lhs, extent const& rhs) noexcept
    {
        return extent{max(static_cast<array_type>(lhs), static_cast<array_type>(rhs))};
    }

    [[nodiscard]] constexpr friend extent clamp(extent const& value, extent const& min, extent const& max) noexcept
    {
        return extent{clamp(static_cast<array_type>(value), static_cast<array_type>(min), static_cast<array_type>(max))};
    }

    /** Check if the extent is valid.
     * Extends must be positive.
     * This function will check if w is zero, and with 2D extent is z is zero.
     */
    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.x() >= value_type{0} && _v.y() >= value_type{0} && _v.z() >= value_type{0} && _v.w() == value_type{0} &&
            (D == 3 || _v.z() == value_type{0});
    }

    [[nodiscard]] friend std::string to_string(extent const& rhs) noexcept
    {
        if constexpr (D == 2) {
            return std::format("[{}, {}]", rhs._v.x(), rhs._v.y());
        } else if constexpr (D == 3) {
            return std::format("[{}, {}, {}]", rhs._v.x(), rhs._v.y(), rhs._v.z());
        } else {
            hi_static_no_default();
        }
    }

    friend std::ostream& operator<<(std::ostream& lhs, extent const& rhs) noexcept
    {
        return lhs << to_string(rhs);
    }

private:
    array_type _v;

    static constexpr std::size_t element_mask = (1_uz << D) - 1;
};

} // namespace geo

/** A 2D extent.
 * @ingroup geometry
 */
using extent2 = geo::extent<float, 2>;

/** A 3D extent.
 * @ingroup geometry
 */
using extent3 = geo::extent<float, 3>;

/** A 2D extent.
 * @ingroup geometry
 */
using extent2i = geo::extent<int, 2>;

/** A 3D extent.
 * @ingroup geometry
 */
using extent3i = geo::extent<int, 3>;

template<>
[[nodiscard]] constexpr extent2i narrow_cast(extent2 const& rhs) noexcept
{
    return {narrow_cast<int>(rhs.width()), narrow_cast<int>(rhs.height())};
}

template<>
[[nodiscard]] constexpr extent2 narrow_cast(extent2i const& rhs) noexcept
{
    return {narrow_cast<float>(rhs.width()), narrow_cast<float>(rhs.height())};
}

}} // namespace hi::v1

template<typename CharT>
struct std::formatter<hi::geo::extent<float, 2>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::extent<float, 2> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "[{}, {}]", std::make_format_args(t.width(), t.height()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::extent<float, 3>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::extent<float, 3> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "[{}, {}, {}]", std::make_format_args(t.width(), t.height(), t.depth()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::extent<int, 2>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::extent<int, 2> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "[{}, {}]", std::make_format_args(t.width(), t.height()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::extent<int, 3>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::extent<int, 3> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "[{}, {}, {}]", std::make_format_args(t.width(), t.height(), t.depth()));
    }
};
