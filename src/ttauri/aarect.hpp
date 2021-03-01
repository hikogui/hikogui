// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "numeric_array.hpp"
#include "alignment.hpp"
#include "concepts.hpp"
#include "geometry/extent.hpp"
#include "geometry/point.hpp"
#include <concepts>

namespace tt {

/** Class which represents an axis-aligned rectangle.
 */
template<typename T>
class axis_aligned_rectangle {
private:
    friend class sfloat_rgba32;

    /** Intrinsic of the rectangle.
     * Elements are assigned as follows:
     *  - (x, y) 2D-coordinate of left-bottom corner of the rectangle
     *  - (z, w) 2D-coordinate of right-top corner of the rectangle
     */
    numeric_array<T, 4> v;

public:
    axis_aligned_rectangle() noexcept : v() {}
    axis_aligned_rectangle(axis_aligned_rectangle const &rhs) noexcept = default;
    axis_aligned_rectangle &operator=(axis_aligned_rectangle const &rhs) noexcept = default;
    axis_aligned_rectangle(axis_aligned_rectangle &&rhs) noexcept = default;
    axis_aligned_rectangle &operator=(axis_aligned_rectangle &&rhs) noexcept = default;

    template<typename O>
    axis_aligned_rectangle(axis_aligned_rectangle<O> const &rhs) noexcept requires(is_different_v<O, T>) :
        axis_aligned_rectangle(rhs.x(), rhs.y(), rhs.width(), rhs.height())
    {
    }

    /** Create a box from the position and size.
     *
     * @param x The x location of the left-bottom corner of the box
     * @param y The y location of the left-bottom corner of the box
     * @param width The width of the box.
     * @param height The height of the box.
     */
    axis_aligned_rectangle(
        tt::arithmetic auto x,
        tt::arithmetic auto y,
        tt::arithmetic auto width,
        tt::arithmetic auto height) noexcept :
        v({narrow_cast<T>(x),
           narrow_cast<T>(y),
           narrow_cast<T>(x) + narrow_cast<T>(width),
           narrow_cast<T>(y) + narrow_cast<T>(height)})
    {
    }

    /** Create a box from the position and size.
     *
     * @param width The width of the box.
     * @param height The height of the box.
     */
    axis_aligned_rectangle(tt::arithmetic auto width, tt::arithmetic auto height) noexcept :
        v(numeric_array<T, 4>(0.0f, 0.0f, narrow_cast<T>(width), narrow_cast<T>(height)))
    {
    }

    /** Create a rectangle from the position and size.
     *
     * @param position The position of the left-bottom corner of the box
     * @param extent The size of the box.
     */
    axis_aligned_rectangle(numeric_array<T, 4> const &position, numeric_array<T, 4> const &extent) noexcept :
        v(position.xyxy() + extent._00xy())
    {
        tt_axiom(position.is_point());
        tt_axiom(position.z() == 0.0);
        tt_axiom(extent.is_vector());
        tt_axiom(extent.z() == 0.0);
    }

    /** Create a rectangle from the size.
     * The rectangle's left bottom corner is at the origin.
     * @param extent The size of the box.
     */
    explicit axis_aligned_rectangle(extent2 const &extent) noexcept : v(static_cast<f32x4>(extent)._00xy()) {}

    /** Create a rectangle from the left-bottom and right-top points.
     * @param p0 The left bottom point.
     * @param p3 The right opt point.
     */
    explicit axis_aligned_rectangle(point2 const &p0, point2 const &p3) noexcept :
        v(static_cast<f32x4>(p0).xy00() + static_cast<f32x4>(p3)._00xy())
    {
        tt_axiom(p0.is_valid());
        tt_axiom(p3.is_valid());
        tt_axiom(p0.x() <= p3.x());
        tt_axiom(p0.y() <= p3.y());
    }

    /** Create a rectangle from the size.
     * The rectangle's left bottom corner is at the origin.
     * @param extent The size of the box.
     */
    explicit axis_aligned_rectangle(numeric_array<T, 4> const &extent) noexcept : v(extent._00xy())
    {
        tt_axiom(extent.is_vector());
        tt_axiom(extent.z() == 0.0);
    }

    /** Create axis_aligned_rectangle from packed p0p3 coordinates.
     * @param v p0 = (x, y), p3 = (z, w)
     */
    [[nodiscard]] static axis_aligned_rectangle p0p3(numeric_array<T, 4> const &v) noexcept
    {
        axis_aligned_rectangle r;
        r.v = v;
        return r;
    }

    /** Create axis_aligned_rectangle from two oposite points
     * @param p0 The left bottom corner.
     * @param p3 The right top corner.
     */
    [[nodiscard]] static axis_aligned_rectangle p0p3(numeric_array<T, 4> const &p0, numeric_array<T, 4> const &p3) noexcept
    {
        tt_axiom(p0.is_point());
        tt_axiom(p3.is_point());
        return axis_aligned_rectangle::p0p3(p0.xy00() + p3._00xy());
    }

    [[nodiscard]] static axis_aligned_rectangle infinity() noexcept requires(std::floating_point<T>)
    {
        return axis_aligned_rectangle::p0p3(numeric_array<T, 4>{
            -std::numeric_limits<T>::infinity(),
            -std::numeric_limits<T>::infinity(),
            std::numeric_limits<T>::infinity(),
            std::numeric_limits<T>::infinity()});
    }

    /** Make sure p0 is left/bottom from p3.
     * @return True is p0 is left and below p3.
     */
    [[nodiscard]] bool valid() const noexcept
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

    /** Expand the current rectangle to include the new point.
     * This is mostly used for extending bounding a bounding box.
     *
     * @param rhs The new rectangle to include in the current rectangle.
     */
    axis_aligned_rectangle &operator|=(numeric_array<T, 4> const &rhs) noexcept
    {
        return *this = *this | rhs;
    }

    /** Translate the box to a new position.
     *
     * @param rhs The vector to add to the coordinates of the rectangle.
     */
    axis_aligned_rectangle &operator+=(numeric_array<T, 4> const &rhs) noexcept
    {
        return *this = *this + rhs;
    }

    /** Translate the box to a new position.
     *
     * @param rhs The vector to subtract from the coordinates of the rectangle.
     */
    axis_aligned_rectangle &operator-=(numeric_array<T, 4> const &rhs) noexcept
    {
        return *this = *this - rhs;
    }

    /** Scale the box by moving the positions (scaling the vectors).
     *
     * @param rhs By how much to scale the positions of the two points
     */
    axis_aligned_rectangle &operator*=(T rhs) noexcept
    {
        return *this = *this * rhs;
    }

    /** Get coordinate of a corner.
     *
     * @param I Corner number: 0 = left-bottom, 1 = right-bottom, 2 = left-top, 3 = right-top.
     * @return The homogeneous coordinate of the corner.
     */
    template<size_t I>
    [[nodiscard]] numeric_array<T, 4> corner() const noexcept
    {
        static_assert(I <= 3);
        if constexpr (I == 0) {
            return v.xy01();
        } else if constexpr (I == 1) {
            return v.zy01();
        } else if constexpr (I == 2) {
            return v.xw01();
        } else {
            return v.zw01();
        }
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

    [[nodiscard]] numeric_array<T, 4> p0() const noexcept
    {
        return corner<0>();
    }

    [[nodiscard]] numeric_array<T, 4> p3() const noexcept
    {
        return corner<3>();
    }

    /** Get vector from origin to the bottom-left corner
     *
     * @return The homogeneous coordinate of the bottom-left corner.
     */
    [[nodiscard]] vector2 offset() const noexcept
    {
        return vector2{v.xy00()};
    }

    /** Get size of the rectangle
     *
     * @return The (x, y) vector representing the width and height of the rectangle.
     */
    [[nodiscard]] extent2 extent() const noexcept
    {
        return extent2{v.zwzw() - v};
    }

    [[nodiscard]] T x() const noexcept
    {
        return v.x();
    }

    [[nodiscard]] T y() const noexcept
    {
        return v.y();
    }

    [[nodiscard]] T width() const noexcept
    {
        return (v.zwzw() - v).x();
    }

    [[nodiscard]] T height() const noexcept
    {
        return (v.zwzw() - v).y();
    }

    [[nodiscard]] T bottom() const noexcept
    {
        return v.y();
    }

    [[nodiscard]] T top() const noexcept
    {
        return v.w();
    }

    [[nodiscard]] T left() const noexcept
    {
        return v.x();
    }

    [[nodiscard]] T right() const noexcept
    {
        return v.z();
    }

    /** The middle on the y-axis between bottom and top.
     */
    [[nodiscard]] T middle() const noexcept
    {
        return (bottom() + top()) * 0.5f;
    }

    /** The center on the x-axis between left and right.
     */
    [[nodiscard]] T center() const noexcept
    {
        return (left() + right()) * 0.5f;
    }

    axis_aligned_rectangle &set_width(T newWidth) noexcept
    {
        v = v.xyxw() + numeric_array<T, 4>{0.0f, 0.0f, newWidth, 0.0f};
        return *this;
    }

    axis_aligned_rectangle &set_height(T newHeight) noexcept
    {
        v = v.xyzy() + numeric_array<T, 4>{0.0f, 0.0f, 0.0f, newHeight};
        return *this;
    }

    /** Check if a 2D coordinate is inside the rectangle.
     *
     * @param rhs The coordinate of the point to test.
     */
    [[nodiscard]] bool contains(numeric_array<T, 4> const &rhs) const noexcept
    {
        // No need to check with empty due to half open range check.
        return ge(rhs.xyxy(), v) == 0b0011;
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
        T x;
        if (alignment == horizontal_alignment::left) {
            x = haystack.p0().x();

        } else if (alignment == horizontal_alignment::right) {
            x = haystack.p3().x() - needle.width();

        } else if (alignment == horizontal_alignment::center) {
            x = (haystack.p0().x() + (haystack.width() * 0.5f)) - (needle.width() * 0.5f);

        } else {
            tt_no_default();
        }

        T y;
        if (alignment == vertical_alignment::bottom) {
            y = haystack.p0().y();

        } else if (alignment == vertical_alignment::top) {
            y = haystack.p3().y() - needle.height();

        } else if (alignment == vertical_alignment::middle) {
            y = (haystack.p0().y() + (haystack.height() * 0.5f)) - (needle.height() * 0.5f);

        } else {
            tt_no_default();
        }

        return {numeric_array<T, 4>::point({x, y}), static_cast<f32x4>(needle.extent())};
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
            return axis_aligned_rectangle::p0p3(min(lhs.p0(), rhs.p0()), max(lhs.p3(), rhs.p3()));
        }
    }

    [[nodiscard]] friend axis_aligned_rectangle
    operator|(axis_aligned_rectangle const &lhs, numeric_array<T, 4> const &rhs) noexcept
    {
        tt_axiom(rhs.is_point());
        if (!lhs) {
            return axis_aligned_rectangle::p0p3(rhs, rhs);
        } else {
            return axis_aligned_rectangle::p0p3(min(lhs.p0(), rhs), max(lhs.p3(), rhs));
        }
    }

    [[nodiscard]] friend axis_aligned_rectangle
    operator+(axis_aligned_rectangle const &lhs, numeric_array<T, 4> const &rhs) noexcept
    {
        return axis_aligned_rectangle::p0p3(lhs.v + rhs.xyxy());
    }

    [[nodiscard]] friend axis_aligned_rectangle
    operator-(axis_aligned_rectangle const &lhs, numeric_array<T, 4> const &rhs) noexcept
    {
        return axis_aligned_rectangle::p0p3(lhs.v - rhs.xyxy());
    }

    [[nodiscard]] friend axis_aligned_rectangle operator*(axis_aligned_rectangle const &lhs, T rhs) noexcept
    {
        return axis_aligned_rectangle::p0p3(lhs.v * rhs);
    }

    /** Get the center of the rectangle.
     */
    [[nodiscard]] friend numeric_array<T, 4> center(axis_aligned_rectangle const &rhs) noexcept
    {
        return (rhs.p0() + rhs.p3()) * 0.5f;
    }

    /** Expand the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much the width and height should be scaled by.
     * @return A new rectangle expanded on each side.
     */
    [[nodiscard]] friend axis_aligned_rectangle scale(axis_aligned_rectangle const &lhs, T rhs) noexcept
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
    [[nodiscard]] friend axis_aligned_rectangle expand(axis_aligned_rectangle const &lhs, T rhs) noexcept
    {
        return axis_aligned_rectangle::p0p3(lhs.v + neg<0b0011>(numeric_array<T, 4>{rhs, rhs, rhs, rhs}));
    }

    /** Expand the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle expanded on each side.
     */
    [[nodiscard]] friend axis_aligned_rectangle expand(axis_aligned_rectangle const &lhs, numeric_array<T, 4> rhs) noexcept
    {
        return axis_aligned_rectangle::p0p3(lhs.v + neg<1, 1, 0, 0>(rhs.xyxy()));
    }

    /** Shrink the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle shrank on each side.
     */
    [[nodiscard]] friend axis_aligned_rectangle shrink(axis_aligned_rectangle const &lhs, T rhs) noexcept
    {
        return expand(lhs, -rhs);
    }

    /** Shrink the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle shrank on each side.
     */
    [[nodiscard]] friend axis_aligned_rectangle shrink(axis_aligned_rectangle const &lhs, numeric_array<T, 4> rhs) noexcept
    {
        return expand(lhs, -rhs);
    }

    [[nodiscard]] friend axis_aligned_rectangle round(axis_aligned_rectangle const &rhs) noexcept
    {
        return axis_aligned_rectangle::p0p3(round(rhs.v));
    }

    /** Round rectangle by expanding to pixel edge.
     */
    [[nodiscard]] friend axis_aligned_rectangle ceil(axis_aligned_rectangle const &rhs) noexcept
    {
        auto p0 = floor(rhs.p0());
        auto p3 = ceil(rhs.p3());
        return axis_aligned_rectangle::p0p3(p0, p3);
    }

    /** Round rectangle by shrinking to pixel edge.
     */
    [[nodiscard]] friend axis_aligned_rectangle floor(axis_aligned_rectangle const &rhs) noexcept
    {
        auto p0 = ceil(rhs.p0());
        auto p3 = floor(rhs.p3());
        return axis_aligned_rectangle::p0p3(p0, p3);
    }

    /** Return the overlapping part of two rectangles.
     * When the rectangles are not overlapping, the width and height are zero.
     */
    [[nodiscard]] friend axis_aligned_rectangle
    intersect(axis_aligned_rectangle const &lhs, axis_aligned_rectangle const &rhs) noexcept
    {
        ttlet p0 = max(lhs.p0(), rhs.p0());
        ttlet p3 = max(p0, min(lhs.p3(), rhs.p3()));
        return axis_aligned_rectangle::p0p3(p0, p3);
    }

    /** Make a rectangle fit inside bounds.
     * This algorithm will try to first move the rectangle and resist resizing it.
     *
     * @param bounds The bounding box.
     * @param rectangle The rectangle to fit inside the bounds.
     * @return A rectangle that fits inside the bounds
     */
    [[nodiscard]] friend axis_aligned_rectangle
    fit(axis_aligned_rectangle const &bounds, axis_aligned_rectangle const &rectangle) noexcept
    {
        ttlet resized_rectangle = axis_aligned_rectangle{
            rectangle.x(),
            rectangle.y(),
            std::min(rectangle.width(), bounds.width()),
            std::min(rectangle.height(), bounds.height())};

        ttlet translate_from_p0 = max(numeric_array<T, 4>{}, bounds.p0() - resized_rectangle.p0());
        ttlet translate_from_p3 = min(numeric_array<T, 4>{}, bounds.p3() - resized_rectangle.p3());
        return resized_rectangle + (translate_from_p0 + translate_from_p3);
    }
};

using aarect = axis_aligned_rectangle<float>;
using iaarect = axis_aligned_rectangle<int>;

} // namespace tt
