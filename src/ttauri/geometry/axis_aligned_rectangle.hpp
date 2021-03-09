// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "numeric_array.hpp"
#include "../alignment.hpp"
#include "../concepts.hpp"
#include "extent.hpp"
#include "point.hpp"
#include <concepts>

namespace tt {

/** Class which represents an axis-aligned rectangle.
 */
class axis_aligned_rectangle {
private:
    friend class sfloat_rgba32;

    /** Intrinsic of the rectangle.
     * Elements are assigned as follows:
     *  - (x, y) 2D-coordinate of left-bottom corner of the rectangle
     *  - (z, w) 2D-coordinate of right-top corner of the rectangle
     */
    f32x4 v;

public:
    axis_aligned_rectangle() noexcept : v() {}
    axis_aligned_rectangle(axis_aligned_rectangle const &rhs) noexcept = default;
    axis_aligned_rectangle &operator=(axis_aligned_rectangle const &rhs) noexcept = default;
    axis_aligned_rectangle(axis_aligned_rectangle &&rhs) noexcept = default;
    axis_aligned_rectangle &operator=(axis_aligned_rectangle &&rhs) noexcept = default;

    explicit axis_aligned_rectangle(f32x4 const &other) noexcept : v(other)
    {
        tt_axiom(is_valid());
    }

    /** Create a box from the position and size.
     *
     * @param x The x location of the left-bottom corner of the box
     * @param y The y location of the left-bottom corner of the box
     * @param width The width of the box.
     * @param height The height of the box.
     */
    axis_aligned_rectangle(float x, float y, float width, float height) noexcept : v{x, y, x + width, y + height}
    {
        tt_axiom(is_valid());
    }

    /** Create a rectangle from the size.
     * The rectangle's left bottom corner is at the origin.
     * @param extent The size of the box.
     */
    explicit axis_aligned_rectangle(extent2 const &extent) noexcept : v(static_cast<f32x4>(extent)._00xy())
    {
        tt_axiom(is_valid());
    }

    /** Create a rectangle from the left-bottom and right-top points.
     * @param p0 The left bottom point.
     * @param p3 The right opt point.
     */
    axis_aligned_rectangle(point2 const &p0, point2 const &p3) noexcept :
        v(static_cast<f32x4>(p0).xy00() + static_cast<f32x4>(p3)._00xy())
    {
        tt_axiom(p0.is_valid());
        tt_axiom(p3.is_valid());
        tt_axiom(is_valid());
    }

    /** Create a rectangle from the size.
     * The rectangle's left bottom corner is at the origin.
     * @param extent The size of the box.
     */
    axis_aligned_rectangle(point2 const &p0, extent2 const &extent) noexcept :
        v(static_cast<f32x4>(p0).xyxy() + static_cast<f32x4>(extent)._00xy())
    {
        tt_axiom(is_valid());
    }

    /** Make sure p0 is left/bottom from p3.
     * @return True is p0 is left and below p3.
     */
    [[nodiscard]] bool is_valid() const noexcept
    {
        return le(v, v.zwzw()) == 0b1111;
    }

    /** Check if the rectangle has no area.
     */
    [[nodiscard]] bool empty() const noexcept
    {
        return eq(v, v.zwxy()) == 0b1111;
    }

    /** True when the rectangle has an area.
     */
    [[nodiscard]] operator bool() const noexcept
    {
        return !empty();
    }

    /** Expand the current rectangle to include the new rectangle.
     * This is mostly used for extending bounding a bounding box.
     *
     * @param rhs The new rectangle to include in the current rectangle.
     */
    axis_aligned_rectangle &operator|=(axis_aligned_rectangle const &rhs) noexcept
    {
        return *this = *this | rhs;
    }

    /** Expand the current rectangle to include the new rectangle.
     * This is mostly used for extending bounding a bounding box.
     *
     * @param rhs A new point to include in the current rectangle.
     */
    axis_aligned_rectangle &operator|=(point2 const &rhs) noexcept
    {
        return *this = *this | rhs;
    }

    [[nodiscard]] constexpr point2 operator[](size_t i) const noexcept
    {
        switch (i) {
        case 0: return point2{v.xy01()};
        case 1: return point2{v.zy01()};
        case 2: return point2{v.xw01()};
        case 3: return point2{v.zw01()};
        default: tt_no_default();
        }
    }

    template<int I>
    [[nodiscard]] constexpr friend point2 get(axis_aligned_rectangle const &rhs) noexcept
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
            tt_static_no_default();
        }
    }

    /** Get size of the rectangle
     *
     * @return The (x, y) vector representing the width and height of the rectangle.
     */
    [[nodiscard]] extent2 extent() const noexcept
    {
        return extent2{v.zwzw() - v};
    }

    [[nodiscard]] float width() const noexcept
    {
        return (v.zwzw() - v).x();
    }

    [[nodiscard]] float height() const noexcept
    {
        return (v.zwzw() - v).y();
    }

    [[nodiscard]] float bottom() const noexcept
    {
        return v.y();
    }

    [[nodiscard]] float top() const noexcept
    {
        return v.w();
    }

    [[nodiscard]] float left() const noexcept
    {
        return v.x();
    }

    [[nodiscard]] float right() const noexcept
    {
        return v.z();
    }

    /** The middle on the y-axis between bottom and top.
     */
    [[nodiscard]] float middle() const noexcept
    {
        return (bottom() + top()) * 0.5f;
    }

    /** The center on the x-axis between left and right.
     */
    [[nodiscard]] float center() const noexcept
    {
        return (left() + right()) * 0.5f;
    }

    axis_aligned_rectangle &set_width(float newWidth) noexcept
    {
        v = v.xyxw() + f32x4{0.0f, 0.0f, newWidth, 0.0f};
        return *this;
    }

    axis_aligned_rectangle &set_height(float newHeight) noexcept
    {
        v = v.xyzy() + f32x4{0.0f, 0.0f, 0.0f, newHeight};
        return *this;
    }

    /** Check if a 2D coordinate is inside the rectangle.
     *
     * @param rhs The coordinate of the point to test.
     */
    [[nodiscard]] bool contains(point2 const &rhs) const noexcept
    {
        // No need to check with empty due to half open range check.
        return ge(static_cast<f32x4>(rhs).xyxy(), v) == 0b0011;
    }

    /** Align a rectangle within another rectangle.
     * @param haystack The outside rectangle
     * @param needle The inside rectangle; to be aligned.
     * @param alignment How the inside rectangle should be aligned.
     * @return The needle rectangle repositioned and aligned inside the haystack.
     */
    [[nodiscard]] friend axis_aligned_rectangle
    align(axis_aligned_rectangle haystack, axis_aligned_rectangle needle, alignment alignment) noexcept
    {
        float x;
        if (alignment == horizontal_alignment::left) {
            x = haystack.left();

        } else if (alignment == horizontal_alignment::right) {
            x = haystack.right() - needle.width();

        } else if (alignment == horizontal_alignment::center) {
            x = haystack.center() - needle.width() * 0.5f;

        } else {
            tt_no_default();
        }

        float y;
        if (alignment == vertical_alignment::bottom) {
            y = haystack.bottom();

        } else if (alignment == vertical_alignment::top) {
            y = haystack.top() - needle.height();

        } else if (alignment == vertical_alignment::middle) {
            y = haystack.middle() - needle.height() * 0.5f;

        } else {
            tt_no_default();
        }

        return {point2{x, y}, needle.extent()};
    }

    /** Need to call the hiden friend function from within another class.
     */
    [[nodiscard]] static axis_aligned_rectangle
    _align(axis_aligned_rectangle outside, axis_aligned_rectangle inside, alignment alignment) noexcept
    {
        return align(outside, inside, alignment);
    }

    [[nodiscard]] friend bool operator==(axis_aligned_rectangle const &lhs, axis_aligned_rectangle const &rhs) noexcept
    {
        return lhs.v == rhs.v;
    }

    [[nodiscard]] friend bool operator!=(axis_aligned_rectangle const &lhs, axis_aligned_rectangle const &rhs) noexcept
    {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend bool overlaps(axis_aligned_rectangle const &lhs, axis_aligned_rectangle const &rhs) noexcept
    {
        if (lhs.empty() || rhs.empty()) {
            return false;
        }

        ttlet rhs_swap = rhs.v.zwxy();

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

    [[nodiscard]] friend axis_aligned_rectangle
    operator|(axis_aligned_rectangle const &lhs, axis_aligned_rectangle const &rhs) noexcept
    {
        if (!lhs) {
            return rhs;
        } else if (!rhs) {
            return lhs;
        } else {
            return axis_aligned_rectangle{min(get<0>(lhs), get<0>(rhs)), max(get<3>(lhs), get<3>(rhs))};
        }
    }

    [[nodiscard]] friend axis_aligned_rectangle operator|(axis_aligned_rectangle const &lhs, point2 const &rhs) noexcept
    {
        if (!lhs) {
            return axis_aligned_rectangle{rhs, rhs};
        } else {
            return axis_aligned_rectangle{min(get<0>(lhs), rhs), max(get<3>(lhs), rhs)};
        }
    }

    /** Get the center of the rectangle.
     */
    [[nodiscard]] friend point2 center(axis_aligned_rectangle const &rhs) noexcept
    {
        return get<0>(rhs) + (get<3>(rhs) - get<0>(rhs)) * 0.5f;
    }

    /** Expand the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much the width and height should be scaled by.
     * @return A new rectangle expanded on each side.
     */
    [[nodiscard]] friend axis_aligned_rectangle scale(axis_aligned_rectangle const &lhs, float rhs) noexcept
    {
        ttlet extent = lhs.extent();
        ttlet scaled_extent = extent * rhs;
        ttlet diff_extent = scaled_extent - extent;
        ttlet half_diff_extent = diff_extent * 0.5f;

        ttlet p1 = get<0>(lhs) - vector2{half_diff_extent};
        ttlet p2 = get<3>(lhs) + vector2{half_diff_extent};
        return axis_aligned_rectangle{p1, p2};
    }

    /** Expand the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle expanded on each side.
     */
    [[nodiscard]] friend axis_aligned_rectangle expand(axis_aligned_rectangle const &lhs, float rhs) noexcept
    {
        return axis_aligned_rectangle{lhs.v + neg<0b0011>(f32x4::broadcast(rhs))};
    }

    /** Shrink the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle shrank on each side.
     */
    [[nodiscard]] friend axis_aligned_rectangle shrink(axis_aligned_rectangle const &lhs, float rhs) noexcept
    {
        return expand(lhs, -rhs);
    }

    [[nodiscard]] friend axis_aligned_rectangle round(axis_aligned_rectangle const &rhs) noexcept
    {
        auto p0 = round(get<0>(rhs));
        auto p3 = round(get<3>(rhs));
        return axis_aligned_rectangle{p0, p3};
    }

    /** Round rectangle by expanding to pixel edge.
     */
    [[nodiscard]] friend axis_aligned_rectangle ceil(axis_aligned_rectangle const &rhs) noexcept
    {
        auto p0 = floor(get<0>(rhs));
        auto p3 = ceil(get<3>(rhs));
        return axis_aligned_rectangle{p0, p3};
    }

    /** Round rectangle by shrinking to pixel edge.
     */
    [[nodiscard]] friend axis_aligned_rectangle floor(axis_aligned_rectangle const &rhs) noexcept
    {
        auto p0 = ceil(get<0>(rhs));
        auto p3 = floor(get<3>(rhs));
        return axis_aligned_rectangle{p0, p3};
    }

    /** Return the overlapping part of two rectangles.
     * When the rectangles are not overlapping, the width and height are zero.
     */
    [[nodiscard]] friend axis_aligned_rectangle
    intersect(axis_aligned_rectangle const &lhs, axis_aligned_rectangle const &rhs) noexcept
    {
        ttlet p0 = max(get<0>(lhs), get<0>(rhs));
        ttlet p3 = min(get<3>(lhs), get<3>(rhs));
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
    [[nodiscard]] friend axis_aligned_rectangle
    fit(axis_aligned_rectangle const &bounds, axis_aligned_rectangle const &rectangle) noexcept;
};

using aarectangle = axis_aligned_rectangle;

} // namespace tt
