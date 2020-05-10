// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/attributes.hpp"

namespace TTauri {

/** Class which represents an axis-aligned rectangle.
 */
class aarect {
    /** Intrinsic of the rectangle.
     * Elements are assigned as follows:
     *  - (x, y) 2D-coordinate of left-bottom corner of the rectangle
     *  - (z, w) 2D-coordinate of right-top corner of the rectangle
     */
    vec v;

public:
    force_inline aarect() noexcept : v() {}
    force_inline aarect(aarect const &rhs) noexcept = default;
    force_inline aarect &operator=(aarect const &rhs) noexcept = default;
    force_inline aarect(aarect &&rhs) noexcept = default;
    force_inline aarect &operator=(aarect &&rhs) noexcept = default;

    aarect(__m128 rhs) noexcept :
        v(rhs) {}

    aarect &operator=(__m128 rhs) noexcept {
        v = rhs;
        return *this;
    }

    operator __m128 () const noexcept {
        return v;
    }

    /** Create a box from the position and size.
     *
     * @param x The x location of the left-bottom corner of the box
     * @param y The y location of the left-bottom corner of the box
     * @param width The width of the box.
     * @param height The height of the box.
     */
    template<typename X, typename Y, typename W=float, typename H=float,
        std::enable_if_t<std::is_arithmetic_v<X> && std::is_arithmetic_v<Y> && std::is_arithmetic_v<W> && std::is_arithmetic_v<H>,int> = 0>
    force_inline aarect(X x, Y y, W width, H height) noexcept :
        aarect(vec(
            numeric_cast<float>(x),
            numeric_cast<float>(y),
            numeric_cast<float>(x) + numeric_cast<float>(width),
            numeric_cast<float>(y) + numeric_cast<float>(height)
        )) {}

    /** Create a box from the position and size.
     *
     * @param offset The position of the left-bottom corner of the box
     * @param extent The size of the box.
     */
    force_inline aarect(vec const &offset, vec const &extent) noexcept :
        aarect(offset.xyxy() + extent._00xy()) {}

    [[nodiscard]] force_inline static aarect p1p2(vec const &p1, vec const &p2) noexcept {
        return _mm_shuffle_ps(p1, p2, _MM_SHUFFLE(1,0,1,0));
    }

    operator bool () const noexcept {
        return v.xyxy() != v.zwzw();
    }

    /** Expand the current rectangle to include the new rectangle.
     * This is mostly used for extending bounding a bounding box.
     *
     * @param rhs The new rectangle to include in the current rectangle.
     */
    aarect &operator|=(aarect const &rhs) noexcept {
        return *this = *this | rhs;
    }

    /** Expand the current rectangle to include the new point.
    * This is mostly used for extending bounding a bounding box.
    *
    * @param rhs The new rectangle to include in the current rectangle.
    */
    aarect &operator|=(vec const &rhs) noexcept {
        return *this = *this | rhs;
    }


    /** Translate the box to a new position.
     *
     * @param rhs The vector to add to the coordinates of the rectangle.
     */
    aarect &operator+=(vec const &rhs) noexcept {
        return *this = *this + rhs;
    }

    /** Translate the box to a new position.
     *
     * @param rhs The vector to subtract from the coordinates of the rectangle.
     */
    aarect &operator-=(vec const &rhs) noexcept {
        return *this = *this - rhs;
    }

    /** Scale the box by moving the positions (scaling the vectors).
    *
    * @param rhs By how much to scale the positions of the two points
    */
    aarect &operator*=(float rhs) noexcept {
        return *this = *this * rhs;
    }

    /** Get coordinate of a corner.
    *
    * @param I Corner number: 0 = left-bottom, 1 = right-bottom, 2 = left-top, 3 = right-top.
    * @return The homogeneous coordinate of the corner.
    */
    template<size_t I>
    [[nodiscard]] force_inline vec corner() const noexcept {
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
    //template<size_t I, typename Z, std::enable_if_t<std::is_arithmetic_v<Z>,int> = 0>
    //[[nodiscard]] force_inline vec corner(Z z) const noexcept {
    //    return corner<I>().z(numeric_cast<float>(z));
    //}

    [[nodiscard]] force_inline vec p1() const noexcept {
        return corner<0>();
    }

    [[nodiscard]] force_inline vec p2() const noexcept {
        return corner<3>();
    }


    /** Get vector from origin to the bottom-left corner
    *
    * @return The homogeneous coordinate of the bottom-left corner.
    */
    [[nodiscard]] force_inline vec offset() const noexcept {
        return v.xy00();
    }

    /** Get vector from origin to the bottom-left corner
    *
    * @param z The z coordinate to insert in the resulting coordinate.
    * @return The homogeneous coordinate of the bottom-left corner.
    */
    //[[nodiscard]] force_inline vec offset(float z) const noexcept {
    //    let _000z = _mm_set_ss(z); 
    //    return _mm_insert_ps(v, _000z, 0b00'10'1000);
    //}

    /** Get size of the rectangle
    *
    * @return The (x, y) vector representing the width and height of the rectangle.
    */
    [[nodiscard]] vec extent() const noexcept {
        return (v.zwzw() - v).xy00();
    }

    [[nodiscard]] force_inline float width() const noexcept {
        return (v.zwzw() - v).x();
    }

    [[nodiscard]] force_inline float height() const noexcept {
        return (v.zwzw() - v).y();
    }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    force_inline aarect &width(T newWidth) noexcept {
        v = v.xyxw() + vec::make_z(newWidth);
        return *this;
    }

    template<typename T, std::enable_if_t<std::is_arithmetic_v<T>,int> = 0>
    force_inline aarect &height(T newHeight) noexcept {
        v = v.xyzy() + vec::make_w(newHeight);
        return *this;
    }

    /** Check if a 2D coordinate is inside the rectangle.
     *
     * @param rhs The coordinate of the point to test.
     */
    [[nodiscard]] bool contains(vec const &rhs) const noexcept {
        return
            (((rhs >= v) & 0b0011) == 0b0011) &&
            (((rhs.xyxy() <= v) & 0b1100) == 0b1100);
    }

    [[nodiscard]] vec align(vec extent, Alignment alignment) const noexcept {
        float x;
        if (alignment == HorizontalAlignment::Left) {
            x = p1().x();

        } else if (alignment == HorizontalAlignment::Right) {
            x = p2().x() - extent.width();

        } else if (alignment == HorizontalAlignment::Center) {
            x = (p1().x() + (width() * 0.5f)) - (extent.width() * 0.5f);

        } else {
            no_default;
        }

        float y;
        if (alignment == VerticalAlignment::Bottom) {
            y = p1().y();

        } else if (alignment == VerticalAlignment::Top) {
            y = p2().y() - extent.height();

        } else if (alignment == VerticalAlignment::Middle) {
            y = (p1().y() + (height() * 0.5f)) - (extent.height() * 0.5f);

        } else if (alignment == VerticalAlignment::Base) {
            y = (p1().y() + (height() * 0.5f)) - (extent.height() * 0.5f);

        } else {
            no_default;
        }

        return vec{x, y};
    }

    /** Align a box inside this box.
     * This will look only at the extent of the other box.
     */
    [[nodiscard]] aarect align(aarect other, Alignment alignment) const noexcept {
        let extent = other.extent();
        let offset = align(extent, alignment);
        return {offset, extent};
    }

    /** Resize to fit and align a box inside this box
     * @param other The box to resize and align inside this box.
     */
    [[nodiscard]] aarect alignFit(aarect other, Alignment alignment) const noexcept {
        let new_extent = other.extent().resize2DRetainingAspectRatio(extent());
        let offset = align(new_extent, alignment);
        return {offset, new_extent};
    }

    [[nodiscard]] friend bool operator==(aarect const &lhs, aarect const &rhs) noexcept {
        return lhs.v == rhs.v;
    }

    [[nodiscard]] friend bool operator!=(aarect const &lhs, aarect const &rhs) noexcept {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend bool overlaps(aarect const &lhs, aarect const &rhs) noexcept {
        let rhs_swap = rhs.v.zwxy();
        if (((lhs.v > rhs_swap) & 0x0011) != 0) {
            return false;
        }
        if (((lhs.v < rhs_swap) & 0x1100) != 0) {
            return false;
        }
        return true;
    }

    [[nodiscard]] friend aarect operator|(aarect const &lhs, aarect const &rhs) noexcept {
        return _mm_blend_ps(min(lhs.v, rhs.v), max(lhs.v, rhs.v), 0b1100);
    }

    [[nodiscard]] friend aarect operator|(aarect const &lhs, vec const &rhs) noexcept {
        ttauri_assume(rhs.w() == 1.0f);
        return _mm_blend_ps(min(lhs.v, rhs), max(lhs.v, rhs.xyxy()), 0b1100);
    }

    [[nodiscard]] friend aarect operator+(aarect const &lhs, vec const &rhs) noexcept {
        return static_cast<__m128>(lhs.v + rhs.xyxy());
    }

    [[nodiscard]] friend aarect operator-(aarect const &lhs, vec const &rhs) noexcept {
        return static_cast<__m128>(lhs.v - rhs.xyxy());
    }

    [[nodiscard]] friend aarect operator*(aarect const &lhs, float rhs) noexcept {
        return static_cast<__m128>(lhs.v * vec{rhs});
    }


    /** Expand the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle expanded on each side.
     */
    [[nodiscard]] friend aarect expand(aarect const &lhs, float rhs) noexcept {
        let _000r = _mm_set_ss(rhs);
        let _00rr = vec{_mm_permute_ps(_000r, _MM_SHUFFLE(1,1,0,0))};
        let _rr00 = vec{_mm_permute_ps(_000r, _MM_SHUFFLE(0,0,1,1))};
        return static_cast<__m128>((lhs.v - _00rr) + _rr00);
    }

    /** Shrink the rectangle for the same amount in all directions.
    * @param lhs The original rectangle.
    * @param rhs How much should be added on each side of the rectangle,
    *            this value may be zero or negative.
    * @return A new rectangle shrank on each side.
    */
    [[nodiscard]] friend aarect shrink(aarect const &lhs, float rhs) noexcept {
        return expand(lhs, -rhs);
    }

    
};

}

