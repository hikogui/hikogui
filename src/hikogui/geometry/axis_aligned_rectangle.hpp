// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file geometry/axis_aligned_rectangle.hpp
 * @ingroup geometry
 */

#pragma once

#include "alignment.hpp"
#include "extent.hpp"
#include "point.hpp"
#include "../rapid/numeric_array.hpp"
#include "../concepts.hpp"
#include "../unfair_mutex.hpp"
#include "../cast.hpp"
#include "../numbers.hpp"
#include <concepts>
#include <mutex>

namespace hi { inline namespace v1 {
namespace geo {
/** Class which represents an axis-aligned rectangle.
 * @ingroup geometry
 */
template<typename T>
class axis_aligned_rectangle {
public:
    using value_type = T;
    using array_type = numeric_array<value_type, 4>;

    constexpr axis_aligned_rectangle() noexcept : v() {}
    constexpr axis_aligned_rectangle(axis_aligned_rectangle const& rhs) noexcept = default;
    constexpr axis_aligned_rectangle& operator=(axis_aligned_rectangle const& rhs) noexcept = default;
    constexpr axis_aligned_rectangle(axis_aligned_rectangle&& rhs) noexcept = default;
    constexpr axis_aligned_rectangle& operator=(axis_aligned_rectangle&& rhs) noexcept = default;

    /** Create a large axis aligned rectangle.
     */
    [[nodiscard]] constexpr static axis_aligned_rectangle large() noexcept
    {
        return {
            point<value_type, 2>{-large_number_v<value_type>, -large_number_v<value_type>},
            point<value_type, 2>{large_number_v<value_type>, large_number_v<value_type>}};
    }

    constexpr explicit axis_aligned_rectangle(array_type const& other) noexcept : v(other)
    {
        hi_axiom(holds_invariant());
    }

    /** Create a box from the position and size.
     *
     * @param x The x location of the left-bottom corner of the box
     * @param y The y location of the left-bottom corner of the box
     * @param width The width of the box.
     * @param height The height of the box.
     */
    constexpr axis_aligned_rectangle(value_type x, value_type y, value_type width, value_type height) noexcept :
        v{x, y, x + width, y + height}
    {
        hi_axiom(holds_invariant());
    }

    /** Create a rectangle from the size.
     * The rectangle's left bottom corner is at the origin.
     * @param extent The size of the box.
     */
    constexpr explicit axis_aligned_rectangle(extent<value_type, 2> const& extent) noexcept :
        v(static_cast<array_type>(extent)._00xy())
    {
        hi_axiom(holds_invariant());
    }

    /** Create a rectangle from the left-bottom and right-top points.
     * @param p0 The left bottom point.
     * @param p3 The right opt point.
     */
    constexpr axis_aligned_rectangle(point<value_type, 2> const& p0, point<value_type, 2> const& p3) noexcept :
        v(static_cast<array_type>(p0).xy00() + static_cast<array_type>(p3)._00xy())
    {
        hi_axiom(p0.holds_invariant());
        hi_axiom(p3.holds_invariant());
        hi_axiom(holds_invariant());
    }

    /** Create a rectangle from the size.
     *
     * The rectangle's left bottom corner is at the origin.
     *
     * @param p0 The left-bottom point where the rectangle starts.
     * @param extent The size of the rectangle.
     */
    constexpr axis_aligned_rectangle(point<value_type, 2> const& p0, extent<value_type, 2> const& extent) noexcept :
        v(static_cast<array_type>(p0).xyxy() + static_cast<array_type>(extent)._00xy())
    {
        hi_axiom(holds_invariant());
    }

    constexpr explicit operator array_type() const noexcept
    {
        return v;
    }

    /** Make sure p0 is left/bottom from p3.
     * @return True is p0 is left and below p3.
     */
    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return le(v, v.zwzw()) == 0b1111;
    }

    /** Check if the rectangle has no area.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return eq(v, v.zwxy()) == 0b1111;
    }

    /** True when the rectangle has an area.
     */
    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Expand the current rectangle to include the new rectangle.
     * This is mostly used for extending bounding a bounding box.
     *
     * @param rhs The new rectangle to include in the current rectangle.
     */
    constexpr axis_aligned_rectangle& operator|=(axis_aligned_rectangle const& rhs) noexcept
    {
        return *this = *this | rhs;
    }

    /** Expand the current rectangle to include the new rectangle.
     * This is mostly used for extending bounding a bounding box.
     *
     * @param rhs A new point to include in the current rectangle.
     */
    constexpr axis_aligned_rectangle& operator|=(point<value_type, 2> const& rhs) noexcept
    {
        return *this = *this | rhs;
    }

    [[nodiscard]] constexpr point<value_type, 2> operator[](std::size_t i) const noexcept
    {
        switch (i) {
        case 0:
            return point<value_type, 2>{v.xy01()};
        case 1:
            return point<value_type, 2>{v.zy01()};
        case 2:
            return point<value_type, 2>{v.xw01()};
        case 3:
            return point<value_type, 2>{v.zw01()};
        default:
            hi_no_default();
        }
    }

    template<int I>
    [[nodiscard]] constexpr friend point<value_type, 2> get(axis_aligned_rectangle const& rhs) noexcept
    {
        if constexpr (I == 0) {
            return point<value_type, 2>{rhs.v.xy01()};
        } else if constexpr (I == 1) {
            return point<value_type, 2>{rhs.v.zy01()};
        } else if constexpr (I == 2) {
            return point<value_type, 2>{rhs.v.xw01()};
        } else if constexpr (I == 3) {
            return point<value_type, 2>{rhs.v.zw01()};
        } else {
            hi_static_no_default();
        }
    }

    /** Get size of the rectangle
     *
     * @return The (x, y) vector representing the width and height of the rectangle.
     */
    [[nodiscard]] constexpr extent<value_type, 2> size() const noexcept
    {
        return extent<value_type, 2>{v.zwzw() - v};
    }

    [[nodiscard]] constexpr value_type x() const noexcept
    {
        return v.x();
    }

    [[nodiscard]] constexpr value_type y() const noexcept
    {
        return v.y();
    }

    [[nodiscard]] constexpr value_type width() const noexcept
    {
        return (v.zwzw() - v).x();
    }

    [[nodiscard]] constexpr value_type height() const noexcept
    {
        return (v.zwzw() - v).y();
    }

    [[nodiscard]] constexpr value_type bottom() const noexcept
    {
        return v.y();
    }

    [[nodiscard]] constexpr value_type top() const noexcept
    {
        return v.w();
    }

    [[nodiscard]] constexpr value_type left() const noexcept
    {
        return v.x();
    }

    [[nodiscard]] constexpr value_type right() const noexcept
    {
        return v.z();
    }

    /** The middle on the y-axis between bottom and top.
     */
    [[nodiscard]] constexpr value_type middle() const noexcept
    {
        return (bottom() + top()) / value_type{2};
    }

    /** The center on the x-axis between left and right.
     */
    [[nodiscard]] constexpr value_type center() const noexcept
    {
        return (left() + right()) / value_type{2};
    }

    /** Get the center of the rectangle.
     */
    [[nodiscard]] constexpr friend point<value_type, 2> midpoint(axis_aligned_rectangle const& rhs) noexcept
    {
        return midpoint(get<0>(rhs), get<3>(rhs));
    }

    constexpr axis_aligned_rectangle& set_width(value_type newWidth) noexcept
    {
        v = v.xyxw() + array_type{value_type{0}, value_type{0}, newWidth, value_type{0}};
        return *this;
    }

    constexpr axis_aligned_rectangle& set_height(value_type newHeight) noexcept
    {
        v = v.xyzy() + array_type{value_type{0}, value_type{0}, value_type{0}, newHeight};
        return *this;
    }

    /** Check if a 2D coordinate is inside the rectangle.
     *
     * @param rhs The coordinate of the point to test.
     */
    [[nodiscard]] constexpr bool contains(point<value_type, 2> const& rhs) const noexcept
    {
        // No need to check with empty due to half open range check.
        return ge(static_cast<array_type>(rhs).xyxy(), v) == 0b0011;
    }

    /** Check if a 3D coordinate is inside the rectangle.
     *
     * @param rhs The coordinate of the point to test. This point is
     *            converted to 2D by this function.
     */
    [[nodiscard]] constexpr bool contains(point<value_type, 3> const& rhs) const noexcept
    {
        return contains(point<value_type, 2>{rhs});
    }

    /** Align a rectangle within another rectangle.
     * @param haystack The outside rectangle
     * @param needle The size of the rectangle to be aligned.
     * @param alignment How the inside rectangle should be aligned.
     * @return The needle rectangle repositioned and aligned inside the haystack.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle
    align(axis_aligned_rectangle haystack, extent<value_type, 2> needle, alignment alignment) noexcept
    {
        auto x = value_type{0};
        if (alignment == horizontal_alignment::left) {
            x = haystack.left();

        } else if (alignment == horizontal_alignment::right) {
            x = haystack.right() - needle.width();

        } else if (alignment == horizontal_alignment::center) {
            x = haystack.center() - needle.width() / value_type{2};

        } else {
            hi_no_default();
        }

        auto y = value_type{0};
        if (alignment == vertical_alignment::bottom) {
            y = haystack.bottom();

        } else if (alignment == vertical_alignment::top) {
            y = haystack.top() - needle.height();

        } else if (alignment == vertical_alignment::middle) {
            y = haystack.middle() - needle.height() / value_type{2};

        } else {
            hi_no_default();
        }

        return {point<value_type, 2>{x, y}, needle};
    }

    /** Align a rectangle within another rectangle.
     * @param haystack The outside rectangle
     * @param needle The inside rectangle; to be aligned.
     * @param alignment How the inside rectangle should be aligned.
     * @return The needle rectangle repositioned and aligned inside the haystack.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle
    align(axis_aligned_rectangle haystack, axis_aligned_rectangle needle, alignment alignment) noexcept
    {
        return align(haystack, needle.size(), alignment);
    }

    /** Need to call the hidden friend function from within another class.
     */
    [[nodiscard]] static constexpr axis_aligned_rectangle
    _align(axis_aligned_rectangle outside, axis_aligned_rectangle inside, alignment alignment) noexcept
    {
        return align(outside, inside, alignment);
    }

    [[nodiscard]] friend constexpr bool operator==(axis_aligned_rectangle const& lhs, axis_aligned_rectangle const& rhs) noexcept
    {
        return lhs.v == rhs.v;
    }

    [[nodiscard]] friend constexpr bool overlaps(axis_aligned_rectangle const& lhs, axis_aligned_rectangle const& rhs) noexcept
    {
        if (lhs.empty() or rhs.empty()) {
            return false;
        }

        hilet rhs_swap = rhs.v.zwxy();

        // lhs.p0.x > rhs.p3.x | lhs.p0.y > rhs.p3.y
        if ((gt(lhs.v, rhs_swap) & 0b0011) != 0) {
            return false;
        }

        // lhs.p3.x < rhs.p0.x | lhs.p3.y < rhs.p0.y
        if ((lt(lhs.v, rhs_swap) & 0b1100) != 0) {
            return false;
        }

        return true;
    }

    [[nodiscard]] friend constexpr axis_aligned_rectangle
    operator|(axis_aligned_rectangle const& lhs, axis_aligned_rectangle const& rhs) noexcept
    {
        if (!lhs) {
            return rhs;
        } else if (!rhs) {
            return lhs;
        } else {
            return axis_aligned_rectangle{min(get<0>(lhs), get<0>(rhs)), max(get<3>(lhs), get<3>(rhs))};
        }
    }

    [[nodiscard]] friend constexpr axis_aligned_rectangle
    operator|(axis_aligned_rectangle const& lhs, point<value_type, 2> const& rhs) noexcept
    {
        if (!lhs) {
            return axis_aligned_rectangle{rhs, rhs};
        } else {
            return axis_aligned_rectangle{min(get<0>(lhs), rhs), max(get<3>(lhs), rhs)};
        }
    }

    /** Expand the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much the width and height should be scaled by.
     * @return A new rectangle expanded on each side.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle operator*(axis_aligned_rectangle const& lhs, value_type rhs) noexcept
    {
        hilet new_extent = lhs.size() * rhs;
        hilet diff = vector<value_type, 2>{new_extent} - vector<value_type, 2>{lhs.size()};
        hilet offset = diff * 0.5f;

        hilet p0 = get<0>(lhs) - offset;
        hilet p3 = max(get<3>(lhs) + offset, p0);
        return axis_aligned_rectangle{p0, p3};
    }

    /** Expand the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle expanded on each side.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle operator+(axis_aligned_rectangle const& lhs, value_type rhs) noexcept
    {
        return axis_aligned_rectangle{lhs.v + neg<0b0011>(array_type::broadcast(rhs))};
    }

    /** Shrink the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle shrank on each side.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle operator-(axis_aligned_rectangle const& lhs, value_type rhs) noexcept
    {
        return lhs + -rhs;
    }

    [[nodiscard]] friend constexpr axis_aligned_rectangle round(axis_aligned_rectangle const& rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        hilet p0 = round(get<0>(rhs));
        hilet size = round(rhs.size());
        return axis_aligned_rectangle{p0, size};
    }

    /** Round rectangle by expanding to pixel edge.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle ceil(axis_aligned_rectangle const& rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        hilet p0 = floor(get<0>(rhs));
        hilet p3 = ceil(get<3>(rhs));
        return axis_aligned_rectangle{p0, p3};
    }

    /** Round rectangle by expanding to a certain granularity.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle
    ceil(axis_aligned_rectangle const& lhs, extent<value_type, 2> const& rhs) noexcept
    {
        hilet p0 = floor(get<0>(lhs), rhs);
        hilet p3 = ceil(get<3>(lhs), rhs);
        return axis_aligned_rectangle{p0, p3};
    }

    /** Round rectangle by shrinking to pixel edge.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle floor(axis_aligned_rectangle const& rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        hilet p0 = ceil(get<0>(rhs));
        hilet p3 = floor(get<3>(rhs));
        return axis_aligned_rectangle{p0, p3};
    }

    [[nodiscard]] friend constexpr axis_aligned_rectangle bounding_rectangle(axis_aligned_rectangle const& rhs) noexcept
    {
        return rhs;
    }

    /** Return the overlapping part of two rectangles.
     * When the rectangles are not overlapping, the width and height are zero.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle
    intersect(axis_aligned_rectangle const& lhs, axis_aligned_rectangle const& rhs) noexcept
    {
        hilet p0 = max(get<0>(lhs), get<0>(rhs));
        hilet p3 = min(get<3>(lhs), get<3>(rhs));
        if (p0.x() < p3.x() && p0.y() < p3.y()) {
            return {p0, p3};
        } else {
            return {};
        }
    }

    [[nodiscard]] constexpr friend value_type
    distance(axis_aligned_rectangle const& lhs, point<value_type, 2> const& rhs) noexcept
    {
        hilet lhs_ = static_cast<array_type>(lhs);
        hilet rhs_ = static_cast<array_type>(rhs);
        // Only (x,y) of subsequent calculations are valid, (z,w) have garbage values.
        hilet closest_point = max(min(rhs_, lhs_.zwzw()), lhs_);
        hilet v_closest_point = closest_point - rhs_;
        return hypot<0b0011>(v_closest_point);
    }

private:
    /** Intrinsic of the rectangle.
     * Elements are assigned as follows:
     *  - (x, y) 2D-coordinate of left-bottom corner of the rectangle
     *  - (z, w) 2D-coordinate of right-top corner of the rectangle
     */
    array_type v;
};

} // namespace geo

using aarectangle = geo::axis_aligned_rectangle<float>;
using aarectanglei = geo::axis_aligned_rectangle<int>;

/** Make a rectangle fit inside bounds.
 * This algorithm will try to first move the rectangle and resist resizing it.
 *
 * @param bounds The bounding box.
 * @param rectangle The rectangle to fit inside the bounds.
 * @return A rectangle that fits inside the bounds
 */
[[nodiscard]] aarectangle fit(aarectangle const& bounds, aarectangle const& rectangle) noexcept;

/** Make a rectangle fit inside bounds.
 * This algorithm will try to first move the rectangle and resist resizing it.
 *
 * @param bounds The bounding box.
 * @param rectangle The rectangle to fit inside the bounds.
 * @return A rectangle that fits inside the bounds
 */
[[nodiscard]] aarectanglei fit(aarectanglei const& bounds, aarectanglei const& rectangle) noexcept;

template<>
[[nodiscard]] constexpr aarectanglei narrow_cast(aarectangle const& rhs) noexcept
{
    return {narrow_cast<int>(rhs.x()), narrow_cast<int>(rhs.y()), narrow_cast<int>(rhs.width()), narrow_cast<int>(rhs.height())};
}

template<>
[[nodiscard]] constexpr aarectangle narrow_cast(aarectanglei const& rhs) noexcept
{
    return {
        narrow_cast<float>(rhs.x()),
        narrow_cast<float>(rhs.y()),
        narrow_cast<float>(rhs.width()),
        narrow_cast<float>(rhs.height())};
}

}} // namespace hi::v1

template<typename T>
class std::atomic<hi::geo::axis_aligned_rectangle<T>> {
public:
    using value_type = hi::geo::axis_aligned_rectangle<T>;
    static constexpr bool is_always_lock_free = false;

    constexpr atomic() noexcept = default;
    atomic(atomic const&) = delete;
    atomic(atomic&&) = delete;
    atomic& operator=(atomic const&) = delete;
    atomic& operator=(atomic&&) = delete;

    constexpr atomic(value_type const& rhs) noexcept : _value(rhs) {}
    atomic& operator=(value_type const& rhs) noexcept
    {
        store(rhs);
        return *this;
    }

    operator value_type() const noexcept
    {
        return load();
    }

    [[nodiscard]] bool is_lock_free() const noexcept
    {
        return is_always_lock_free;
    }

    void store(value_type desired, std::memory_order = std::memory_order_seq_cst) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        _value = desired;
    }

    value_type load(std::memory_order = std::memory_order_seq_cst) const noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        return _value;
    }

    value_type exchange(value_type desired, std::memory_order = std::memory_order_seq_cst) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        return std::exchange(_value, desired);
    }

    bool compare_exchange_weak(value_type& expected, value_type desired, std::memory_order, std::memory_order) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        if (_value == expected) {
            _value = desired;
            return true;
        } else {
            expected = _value;
            return false;
        }
    }

    bool compare_exchange_strong(
        value_type& expected,
        value_type desired,
        std::memory_order success,
        std::memory_order failure) noexcept
    {
        return compare_exchange_weak(expected, desired, success, failure);
    }

    bool
    compare_exchange_weak(value_type& expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept
    {
        return compare_exchange_weak(expected, desired, order, order);
    }

    bool compare_exchange_strong(
        value_type& expected,
        value_type desired,
        std::memory_order order = std::memory_order_seq_cst) noexcept
    {
        return compare_exchange_strong(expected, desired, order, order);
    }

    value_type fetch_or(value_type arg, std::memory_order = std::memory_order_seq_cst) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        auto tmp = _value;
        _value = tmp | arg;
        return tmp;
    }

    value_type operator|=(value_type arg) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        return _value |= arg;
    }

private:
    value_type _value;
    mutable hi::unfair_mutex _mutex;
};

template<typename CharT>
struct std::formatter<hi::geo::axis_aligned_rectangle<float>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::axis_aligned_rectangle<float> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "{}:{}", std::make_format_args(get<0>(t), t.size()));
    }
};

template<typename CharT>
struct std::formatter<hi::geo::axis_aligned_rectangle<int>, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::geo::axis_aligned_rectangle<int> const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "{}:{}", std::make_format_args(get<0>(t), t.size()));
    }
};
