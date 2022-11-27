// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file geometry/axis_aligned_rectangle.hpp
 * @ingroup geometry
 */

#pragma once

#include "../rapid/numeric_array.hpp"
#include "../alignment.hpp"
#include "../concepts.hpp"
#include "../unfair_mutex.hpp"
#include "extent.hpp"
#include "point.hpp"
#include <concepts>
#include <mutex>

namespace hi {
inline namespace v1 {

/** Class which represents an axis-aligned rectangle.
 * @ingroup geometry
 */
class axis_aligned_rectangle {
private:
    /** Intrinsic of the rectangle.
     * Elements are assigned as follows:
     *  - (x, y) 2D-coordinate of left-bottom corner of the rectangle
     *  - (z, w) 2D-coordinate of right-top corner of the rectangle
     */
    f32x4 v;

public:
    constexpr axis_aligned_rectangle() noexcept : v() {}
    constexpr axis_aligned_rectangle(axis_aligned_rectangle const& rhs) noexcept = default;
    constexpr axis_aligned_rectangle& operator=(axis_aligned_rectangle const& rhs) noexcept = default;
    constexpr axis_aligned_rectangle(axis_aligned_rectangle&& rhs) noexcept = default;
    constexpr axis_aligned_rectangle& operator=(axis_aligned_rectangle&& rhs) noexcept = default;

    constexpr explicit axis_aligned_rectangle(f32x4 const& other) noexcept : v(other)
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
    constexpr axis_aligned_rectangle(float x, float y, float width, float height) noexcept : v{x, y, x + width, y + height}
    {
        hi_axiom(holds_invariant());
    }

    /** Create a rectangle from the size.
     * The rectangle's left bottom corner is at the origin.
     * @param extent The size of the box.
     */
    constexpr explicit axis_aligned_rectangle(extent2 const& extent) noexcept : v(static_cast<f32x4>(extent)._00xy())
    {
        hi_axiom(holds_invariant());
    }

    /** Create a rectangle from the left-bottom and right-top points.
     * @param p0 The left bottom point.
     * @param p3 The right opt point.
     */
    constexpr axis_aligned_rectangle(point2 const& p0, point2 const& p3) noexcept :
        v(static_cast<f32x4>(p0).xy00() + static_cast<f32x4>(p3)._00xy())
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
    constexpr axis_aligned_rectangle(point2 const& p0, extent2 const& extent) noexcept :
        v(static_cast<f32x4>(p0).xyxy() + static_cast<f32x4>(extent)._00xy())
    {
        hi_axiom(holds_invariant());
    }

    constexpr explicit operator f32x4() const noexcept
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
    constexpr axis_aligned_rectangle& operator|=(point2 const& rhs) noexcept
    {
        return *this = *this | rhs;
    }

    [[nodiscard]] constexpr point2 operator[](std::size_t i) const noexcept
    {
        switch (i) {
        case 0:
            return point2{v.xy01()};
        case 1:
            return point2{v.zy01()};
        case 2:
            return point2{v.xw01()};
        case 3:
            return point2{v.zw01()};
        default:
            hi_no_default();
        }
    }

    template<int I>
    [[nodiscard]] constexpr friend point2 get(axis_aligned_rectangle const& rhs) noexcept
    {
        if constexpr (I == 0) {
            return point2{rhs.v.xy01()};
        } else if constexpr (I == 1) {
            return point2{rhs.v.zy01()};
        } else if constexpr (I == 2) {
            return point2{rhs.v.xw01()};
        } else if constexpr (I == 3) {
            return point2{rhs.v.zw01()};
        } else {
            hi_static_no_default();
        }
    }

    /** Get size of the rectangle
     *
     * @return The (x, y) vector representing the width and height of the rectangle.
     */
    [[nodiscard]] constexpr extent2 size() const noexcept
    {
        return extent2{v.zwzw() - v};
    }

    [[nodiscard]] constexpr float width() const noexcept
    {
        return (v.zwzw() - v).x();
    }

    [[nodiscard]] constexpr float height() const noexcept
    {
        return (v.zwzw() - v).y();
    }

    [[nodiscard]] constexpr float bottom() const noexcept
    {
        return v.y();
    }

    [[nodiscard]] constexpr float top() const noexcept
    {
        return v.w();
    }

    [[nodiscard]] constexpr float left() const noexcept
    {
        return v.x();
    }

    [[nodiscard]] constexpr float right() const noexcept
    {
        return v.z();
    }

    /** The middle on the y-axis between bottom and top.
     */
    [[nodiscard]] constexpr float middle() const noexcept
    {
        return (bottom() + top()) * 0.5f;
    }

    /** The center on the x-axis between left and right.
     */
    [[nodiscard]] constexpr float center() const noexcept
    {
        return (left() + right()) * 0.5f;
    }

    /** Get the center of the rectangle.
     */
    [[nodiscard]] constexpr friend point2 midpoint(axis_aligned_rectangle const& rhs) noexcept
    {
        return midpoint(get<0>(rhs), get<3>(rhs));
    }

    constexpr axis_aligned_rectangle& set_width(float newWidth) noexcept
    {
        v = v.xyxw() + f32x4{0.0f, 0.0f, newWidth, 0.0f};
        return *this;
    }

    constexpr axis_aligned_rectangle& set_height(float newHeight) noexcept
    {
        v = v.xyzy() + f32x4{0.0f, 0.0f, 0.0f, newHeight};
        return *this;
    }

    /** Check if a 2D coordinate is inside the rectangle.
     *
     * @param rhs The coordinate of the point to test.
     */
    [[nodiscard]] constexpr bool contains(point2 const& rhs) const noexcept
    {
        // No need to check with empty due to half open range check.
        return ge(static_cast<f32x4>(rhs).xyxy(), v) == 0b0011;
    }

    /** Check if a 3D coordinate is inside the rectangle.
     *
     * @param rhs The coordinate of the point to test. This point is
     *            converted to 2D by this function.
     */
    [[nodiscard]] constexpr bool contains(point3 const& rhs) const noexcept
    {
        return contains(point2{rhs});
    }

    /** Align a rectangle within another rectangle.
     * @param haystack The outside rectangle
     * @param needle The size of the rectangle to be aligned.
     * @param alignment How the inside rectangle should be aligned.
     * @return The needle rectangle repositioned and aligned inside the haystack.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle
    align(axis_aligned_rectangle haystack, extent2 needle, alignment alignment) noexcept
    {
        auto x = 0.0f;
        if (alignment == horizontal_alignment::left) {
            x = haystack.left();

        } else if (alignment == horizontal_alignment::right) {
            x = haystack.right() - needle.width();

        } else if (alignment == horizontal_alignment::center) {
            x = haystack.center() - needle.width() * 0.5f;

        } else {
            hi_no_default();
        }

        auto y = 0.0f;
        if (alignment == vertical_alignment::bottom) {
            y = haystack.bottom();

        } else if (alignment == vertical_alignment::top) {
            y = haystack.top() - needle.height();

        } else if (alignment == vertical_alignment::middle) {
            y = haystack.middle() - needle.height() * 0.5f;

        } else {
            hi_no_default();
        }

        return {point2{x, y}, needle};
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

    [[nodiscard]] friend constexpr axis_aligned_rectangle operator|(axis_aligned_rectangle const& lhs, point2 const& rhs) noexcept
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
    [[nodiscard]] friend constexpr axis_aligned_rectangle operator*(axis_aligned_rectangle const& lhs, float rhs) noexcept
    {
        hilet new_extent = lhs.size() * rhs;
        hilet diff = vector2{new_extent} - vector2{lhs.size()};
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
    [[nodiscard]] friend constexpr axis_aligned_rectangle operator+(axis_aligned_rectangle const& lhs, float rhs) noexcept
    {
        return axis_aligned_rectangle{lhs.v + neg<0b0011>(f32x4::broadcast(rhs))};
    }

    /** Shrink the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle shrank on each side.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle operator-(axis_aligned_rectangle const& lhs, float rhs) noexcept
    {
        return lhs + -rhs;
    }

    [[nodiscard]] friend constexpr axis_aligned_rectangle round(axis_aligned_rectangle const& rhs) noexcept
    {
        hilet p0 = round(get<0>(rhs));
        hilet size = round(rhs.size());
        return axis_aligned_rectangle{p0, size};
    }

    /** Round rectangle by expanding to pixel edge.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle ceil(axis_aligned_rectangle const& rhs) noexcept
    {
        hilet p0 = floor(get<0>(rhs));
        hilet p3 = ceil(get<3>(rhs));
        return axis_aligned_rectangle{p0, p3};
    }

    /** Round rectangle by expanding to a certain granularity.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle ceil(axis_aligned_rectangle const& lhs, extent2 const& rhs) noexcept
    {
        hilet p0 = floor(get<0>(lhs), rhs);
        hilet p3 = ceil(get<3>(lhs), rhs);
        return axis_aligned_rectangle{p0, p3};
    }

    /** Round rectangle by shrinking to pixel edge.
     */
    [[nodiscard]] friend constexpr axis_aligned_rectangle floor(axis_aligned_rectangle const& rhs) noexcept
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

    /** Make a rectangle fit inside bounds.
     * This algorithm will try to first move the rectangle and resist resizing it.
     *
     * @param bounds The bounding box.
     * @param rectangle The rectangle to fit inside the bounds.
     * @return A rectangle that fits inside the bounds
     */
    friend axis_aligned_rectangle fit(axis_aligned_rectangle const& bounds, axis_aligned_rectangle const& rectangle) noexcept;

    [[nodiscard]] constexpr friend float distance(axis_aligned_rectangle const& lhs, point2 const& rhs) noexcept
    {
        hilet lhs_ = static_cast<f32x4>(lhs);
        hilet rhs_ = static_cast<f32x4>(rhs);
        // Only (x,y) of subsequent calculations are valid, (z,w) have garbage values.
        hilet closest_point = max(min(rhs_, lhs_.zwzw()), lhs_);
        hilet v_closest_point = closest_point - rhs_;
        return hypot<0b0011>(v_closest_point);
    }
};

using aarectangle = axis_aligned_rectangle;

}} // namespace hi::inline v1

template<>
class std::atomic<hi::axis_aligned_rectangle> {
public:
    static constexpr bool is_always_lock_free = false;

    constexpr atomic() noexcept = default;
    atomic(atomic const&) = delete;
    atomic(atomic&&) = delete;
    atomic& operator=(atomic const&) = delete;
    atomic& operator=(atomic&&) = delete;

    constexpr atomic(hi::axis_aligned_rectangle const& rhs) noexcept : _value(rhs) {}
    atomic& operator=(hi::axis_aligned_rectangle const& rhs) noexcept
    {
        store(rhs);
        return *this;
    }

    operator hi::axis_aligned_rectangle() const noexcept
    {
        return load();
    }

    [[nodiscard]] bool is_lock_free() const noexcept
    {
        return is_always_lock_free;
    }

    void store(hi::axis_aligned_rectangle desired, std::memory_order = std::memory_order_seq_cst) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        _value = desired;
    }

    hi::axis_aligned_rectangle load(std::memory_order = std::memory_order_seq_cst) const noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        return _value;
    }

    hi::axis_aligned_rectangle
    exchange(hi::axis_aligned_rectangle desired, std::memory_order = std::memory_order_seq_cst) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        return std::exchange(_value, desired);
    }

    bool compare_exchange_weak(
        hi::axis_aligned_rectangle& expected,
        hi::axis_aligned_rectangle desired,
        std::memory_order,
        std::memory_order) noexcept
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
        hi::axis_aligned_rectangle& expected,
        hi::axis_aligned_rectangle desired,
        std::memory_order success,
        std::memory_order failure) noexcept
    {
        return compare_exchange_weak(expected, desired, success, failure);
    }

    bool compare_exchange_weak(
        hi::axis_aligned_rectangle& expected,
        hi::axis_aligned_rectangle desired,
        std::memory_order order = std::memory_order_seq_cst) noexcept
    {
        return compare_exchange_weak(expected, desired, order, order);
    }

    bool compare_exchange_strong(
        hi::axis_aligned_rectangle& expected,
        hi::axis_aligned_rectangle desired,
        std::memory_order order = std::memory_order_seq_cst) noexcept
    {
        return compare_exchange_strong(expected, desired, order, order);
    }

    hi::axis_aligned_rectangle fetch_or(hi::axis_aligned_rectangle arg, std::memory_order = std::memory_order_seq_cst) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        auto tmp = _value;
        _value = tmp | arg;
        return tmp;
    }

    hi::axis_aligned_rectangle operator|=(hi::axis_aligned_rectangle arg) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        return _value |= arg;
    }

private:
    hi::axis_aligned_rectangle _value;
    mutable hi::unfair_mutex _mutex;
};

template<typename CharT>
struct std::formatter<hi::axis_aligned_rectangle, CharT> : std::formatter<float, CharT> {
    auto parse(auto& pc)
    {
        return pc.end();
    }

    auto format(hi::axis_aligned_rectangle const& t, auto& fc)
    {
        return std::vformat_to(fc.out(), "{}:{}", std::make_format_args(get<0>(t), t.size()));
    }
};
