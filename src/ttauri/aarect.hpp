// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "vec.hpp"
#include "alignment.hpp"

namespace tt {

/** Class which represents an axis-aligned rectangle.
 */
class aarect {
private:
    friend class R32G32B32A32SFloat;

    /** Intrinsic of the rectangle.
     * Elements are assigned as follows:
     *  - (x, y) 2D-coordinate of left-bottom corner of the rectangle
     *  - (z, w) 2D-coordinate of right-top corner of the rectangle
     */
    vec v;

public:
    aarect() noexcept : v() {}
    aarect(aarect const &rhs) noexcept = default;
    aarect &operator=(aarect const &rhs) noexcept = default;
    aarect(aarect &&rhs) noexcept = default;
    aarect &operator=(aarect &&rhs) noexcept = default;

    /** Create a box from the position and size.
     *
     * @param x The x location of the left-bottom corner of the box
     * @param y The y location of the left-bottom corner of the box
     * @param width The width of the box.
     * @param height The height of the box.
     */
    template<
        typename X,
        typename Y,
        typename W,
        typename H,
        std::enable_if_t<
            std::is_arithmetic_v<X> && std::is_arithmetic_v<Y> && std::is_arithmetic_v<W> && std::is_arithmetic_v<H>,
            int> = 0>
    aarect(X x, Y y, W width, H height) noexcept :
        v(
            vec(numeric_cast<float>(x),
                numeric_cast<float>(y),
                numeric_cast<float>(x) + numeric_cast<float>(width),
                numeric_cast<float>(y) + numeric_cast<float>(height)))
    {
    }

    /** Create a box from the position and size.
     *
     * @param width The width of the box.
     * @param height The height of the box.
     */
    template<typename W, typename H, std::enable_if_t<std::is_arithmetic_v<W> && std::is_arithmetic_v<H>, int> = 0>
    aarect(W width, H height) noexcept : v(vec(0.0f, 0.0f, numeric_cast<float>(width), numeric_cast<float>(height)))
    {
    }

    /** Create a rectangle from the position and size.
     *
     * @param position The position of the left-bottom corner of the box
     * @param extent The size of the box.
     */
    aarect(vec const &position, vec const &extent) noexcept : v(position.xyxy() + extent._00xy())
    {
        tt_assume(position.is_point());
        tt_assume(position.z() == 0.0);
        tt_assume(extent.is_vector());
        tt_assume(extent.z() == 0.0);
    }

    /** Create a rectangle from the size.
     * The rectangle's left bottom corner is at the origin.
     * @param extent The size of the box.
     */
    explicit aarect(vec const &extent) noexcept : v(extent._00xy())
    {
        tt_assume(extent.is_vector());
        tt_assume(extent.z() == 0.0);
    }

    /** Create aarect from packed p0p3 coordinates.
     * @param v p0 = (x, y), p3 = (z, w)
     */
    [[nodiscard]] static aarect p0p3(vec const &v) noexcept
    {
        aarect r;
        r.v = v;
        return r;
    }

    [[nodiscard]] static aarect p0p3(vec const &p0, vec const &p3) noexcept
    {
        tt_assume(p0.is_point());
        tt_assume(p3.is_point());
        return aarect::p0p3(p0.xy00() + p3._00xy());
    }

    operator bool() const noexcept
    {
        return v.xyxy() != v.zwzw();
    }

    /** Expand the current rectangle to include the new rectangle.
     * This is mostly used for extending bounding a bounding box.
     *
     * @param rhs The new rectangle to include in the current rectangle.
     */
    aarect &operator|=(aarect const &rhs) noexcept
    {
        return *this = *this | rhs;
    }

    /** Expand the current rectangle to include the new point.
     * This is mostly used for extending bounding a bounding box.
     *
     * @param rhs The new rectangle to include in the current rectangle.
     */
    aarect &operator|=(vec const &rhs) noexcept
    {
        return *this = *this | rhs;
    }

    /** Translate the box to a new position.
     *
     * @param rhs The vector to add to the coordinates of the rectangle.
     */
    aarect &operator+=(vec const &rhs) noexcept
    {
        return *this = *this + rhs;
    }

    /** Translate the box to a new position.
     *
     * @param rhs The vector to subtract from the coordinates of the rectangle.
     */
    aarect &operator-=(vec const &rhs) noexcept
    {
        return *this = *this - rhs;
    }

    /** Scale the box by moving the positions (scaling the vectors).
     *
     * @param rhs By how much to scale the positions of the two points
     */
    aarect &operator*=(float rhs) noexcept
    {
        return *this = *this * rhs;
    }

    /** Get coordinate of a corner.
     *
     * @param I Corner number: 0 = left-bottom, 1 = right-bottom, 2 = left-top, 3 = right-top.
     * @return The homogeneous coordinate of the corner.
     */
    template<size_t I>
    [[nodiscard]] vec corner() const noexcept
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

    /** Get coordinate of a corner.
     *
     * @param I Corner number: 0 = left-bottom, 1 = right-bottom, 2 = left-top, 3 = right-top.
     * @param z The z coordinate to insert in the resulting coordinate.
     * @return The homogeneous coordinate of the corner.
     */
    // template<size_t I, typename Z, std::enable_if_t<std::is_arithmetic_v<Z>,int> = 0>
    //[[nodiscard]] vec corner(Z z) const noexcept {
    //    return corner<I>().z(numeric_cast<float>(z));
    //}

    [[nodiscard]] vec p0() const noexcept
    {
        return corner<0>();
    }

    [[nodiscard]] vec p3() const noexcept
    {
        return corner<3>();
    }

    /** Get vector from origin to the bottom-left corner
     *
     * @return The homogeneous coordinate of the bottom-left corner.
     */
    [[nodiscard]] vec offset() const noexcept
    {
        return v.xy00();
    }

    /** Get size of the rectangle
     *
     * @return The (x, y) vector representing the width and height of the rectangle.
     */
    [[nodiscard]] vec extent() const noexcept
    {
        return (v.zwzw() - v).xy00();
    }

    [[nodiscard]] float x() const noexcept
    {
        return v.x();
    }

    [[nodiscard]] float y() const noexcept
    {
        return v.y();
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

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    aarect &width(T newWidth) noexcept
    {
        v = v.xyxw() + vec::make_z(newWidth);
        return *this;
    }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
    aarect &height(T newHeight) noexcept
    {
        v = v.xyzy() + vec::make_w(newHeight);
        return *this;
    }

    /** Check if a 2D coordinate is inside the rectangle.
     *
     * @param rhs The coordinate of the point to test.
     */
    [[nodiscard]] bool contains(vec const &rhs) const noexcept
    {
        return ge(rhs.xyxy(), v) == 0b0011;
    }

    /** Align a rectangle within another rectangle.
     * @param haystack The outside rectangle
     * @param needle The inside rectangle; to be aligned.
     * @param alignment How the inside rectangle should be aligned.
     * @return The needle rectangle repositioned and aligned inside the haystack.
     */
    [[nodiscard]] friend aarect align(aarect haystack, aarect needle, Alignment alignment) noexcept
    {
        float x;
        if (alignment == HorizontalAlignment::Left) {
            x = haystack.p0().x();

        } else if (alignment == HorizontalAlignment::Right) {
            x = haystack.p3().x() - needle.width();

        } else if (alignment == HorizontalAlignment::Center) {
            x = (haystack.p0().x() + (haystack.width() * 0.5f)) - (needle.width() * 0.5f);

        } else {
            tt_no_default;
        }

        float y;
        if (alignment == VerticalAlignment::Bottom) {
            y = haystack.p0().y();

        } else if (alignment == VerticalAlignment::Top) {
            y = haystack.p3().y() - needle.height();

        } else if (alignment == VerticalAlignment::Middle) {
            y = (haystack.p0().y() + (haystack.height() * 0.5f)) - (needle.height() * 0.5f);

        } else {
            tt_no_default;
        }

        return {vec::point(x, y), needle.extent()};
    }

    /** Need to call the hiden friend function from within another class.
     */
    [[nodiscard]] static aarect _align(aarect outside, aarect inside, Alignment alignment) noexcept
    {
        return align(outside, inside, alignment);
    }

    [[nodiscard]] friend bool operator==(aarect const &lhs, aarect const &rhs) noexcept
    {
        return lhs.v == rhs.v;
    }

    [[nodiscard]] friend bool operator!=(aarect const &lhs, aarect const &rhs) noexcept
    {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend bool overlaps(aarect const &lhs, aarect const &rhs) noexcept
    {
        ttlet rhs_swap = rhs.v.zwxy();
        if ((gt(lhs.v, rhs_swap) & 0x0011) != 0) {
            return false;
        }
        if ((lt(lhs.v, rhs_swap) & 0x1100) != 0) {
            return false;
        }
        return true;
    }

    [[nodiscard]] friend aarect operator|(aarect const &lhs, aarect const &rhs) noexcept
    {
        return aarect::p0p3(min(lhs.p0(), rhs.p0()), max(lhs.p3(), rhs.p3()));
    }

    [[nodiscard]] friend aarect operator|(aarect const &lhs, vec const &rhs) noexcept
    {
        tt_assume(rhs.is_point());
        return aarect::p0p3(min(lhs.p0(), rhs), max(lhs.p3(), rhs));
    }

    [[nodiscard]] friend aarect operator+(aarect const &lhs, vec const &rhs) noexcept
    {
        return aarect::p0p3(lhs.v + rhs.xyxy());
    }

    [[nodiscard]] friend aarect operator-(aarect const &lhs, vec const &rhs) noexcept
    {
        return aarect::p0p3(lhs.v - rhs.xyxy());
    }

    [[nodiscard]] friend aarect operator*(aarect const &lhs, float rhs) noexcept
    {
        return aarect::p0p3(lhs.v * vec{rhs});
    }

    /** Get the center of the rectangle.
     */
    [[nodiscard]] friend vec center(aarect const &rhs) noexcept
    {
        return (rhs.p0() + rhs.p3()) * 0.5f;
    }

    /** Expand the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much the width and height should be scaled by.
     * @return A new rectangle expanded on each side.
     */
    [[nodiscard]] friend aarect scale(aarect const &lhs, float rhs) noexcept
    {
        ttlet extent = lhs.extent();
        ttlet scaled_extent = extent * rhs;
        ttlet diff_extent = scaled_extent - extent;
        ttlet half_diff_extent = diff_extent * 0.5f;

        ttlet p1 = lhs.p0() - half_diff_extent;
        ttlet p2 = lhs.p3() + half_diff_extent;
        return aarect::p0p3(p1, p2);
    }

    /** Expand the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle expanded on each side.
     */
    [[nodiscard]] friend aarect expand(aarect const &lhs, float rhs) noexcept
    {
        return aarect::p0p3(lhs.v + neg<1, 1, 0, 0>(vec{rhs}));
    }

    /** Shrink the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle shrank on each side.
     */
    [[nodiscard]] friend aarect shrink(aarect const &lhs, float rhs) noexcept
    {
        return expand(lhs, -rhs);
    }

    [[nodiscard]] friend aarect round(aarect const &rhs) noexcept
    {
        return aarect::p0p3(round(rhs.v));
    }
};

} // namespace tt
