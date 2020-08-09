// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ivec.hpp"

namespace tt {

/** Class which represents an axis-aligned rectangle.
 */
class iaarect {
    /** Intrinsic of the rectangle.
     * Elements are assigned as follows:
     *  - (x, y) 2D-coordinate of left-bottom corner of the rectangle
     *  - (z, w) 2D-coordinate of right-top corner of the rectangle
     */
    ivec v;

public:
    iaarect() noexcept : v() {}
    iaarect(iaarect const &rhs) noexcept = default;
    iaarect &operator=(iaarect const &rhs) noexcept = default;
    iaarect(iaarect &&rhs) noexcept = default;
    iaarect &operator=(iaarect &&rhs) noexcept = default;

    iaarect(__m128i rhs) noexcept :
        v(rhs) {}

    iaarect &operator=(__m128i rhs) noexcept {
        v = rhs;
        return *this;
    }

    operator __m128i () const noexcept {
        return v;
    }

    /** Create a box from the position and size.
     *
     * @param x The x location of the left-bottom corner of the box
     * @param y The y location of the left-bottom corner of the box
     * @param width The width of the box.
     * @param height The height of the box.
     */
    template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
    iaarect(T x, T y, T width, T height) noexcept :
        iaarect(ivec(x, y, x + width, y + height)) {}

    /** Create a box from the position and size.
     *
     * @param offset The position of the left-bottom corner of the box
     * @param extent The size of the box (z and w must be zero).
     */
    iaarect(ivec const &offset, ivec const &extent) noexcept :
        iaarect(offset.xyxy() + extent.zwxy()) {}

    [[nodiscard]] static iaarect p0p3(ivec const &p1, ivec const &p2) noexcept {
        return _mm_blend_epi16(p1, p2.xyxy(), 0b11'11'00'00);
    }

    /** Expand the current rectangle to include the new rectangle.
     * This is mostly used for extending bounding a bounding box.
     *
     * @param rhs The new rectangle to include in the current rectangle.
     */
    iaarect &operator|=(iaarect const &rhs) noexcept {
        return *this = *this | rhs;
    }

    /** Translate the box to a new position.
     *
     * @param rhs The ivector to add to the coordinates of the rectangle.
     */
    iaarect &operator+=(ivec const &rhs) noexcept {
        return *this = *this + rhs;
    }

    /** Translate the box to a new position.
     *
     * @param rhs The ivector to subtract from the coordinates of the rectangle.
     */
    iaarect &operator-=(ivec const &rhs) noexcept {
        return *this = *this - rhs;
    }

    /** Get coordinate of a corner.
    *
    * @param I Corner number: 0 = left-bottom, 1 = right-bottom, 2 = left-top, 3 = right-top.
    * @return The homogenious coordinate of the corner.
    */
    template<size_t I>
    [[nodiscard]] ivec corner() const noexcept {
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

    /** Get coordinate of the bottom-left corner
    *
    * @return The homogenious coordinate of the bottom-left corner.
    */
    [[nodiscard]] ivec offset() const noexcept { return corner<0>(); }

    
    /** Get size of the rectangle
    *
    * @return The (x, y) ivector representing the width and height of the rectangle.
    */
    [[nodiscard]] ivec extent() const noexcept {
        return corner<3>() - corner<0>();
    }

    [[nodiscard]] int x1() const noexcept { return v.x(); } 
    [[nodiscard]] int y1() const noexcept { return v.y(); } 
    [[nodiscard]] int x2() const noexcept { return v.z(); } 
    [[nodiscard]] int y2() const noexcept { return v.w(); } 
    [[nodiscard]] int width() const noexcept { return extent().x(); }
    [[nodiscard]] int height() const noexcept { return extent().y(); }

    /** Check if a 2D coordinate is inside the rectangle.
     *
     * @param rhs The coordinate of the point to test.
     */
    [[nodiscard]] bool contains(ivec const &rhs) const noexcept {
        return
            (((rhs >= v) & 0x00ff) == 0x00ff) &&
            (((rhs <= v) & 0xff00) == 0xff00);
    }

    [[nodiscard]] friend bool operator==(iaarect const &lhs, iaarect const &rhs) noexcept {
        return lhs.v == rhs.v;
    }

    [[nodiscard]] friend bool operator!=(iaarect const &lhs, iaarect const &rhs) noexcept {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend iaarect operator|(iaarect const &lhs, iaarect const &rhs) noexcept {
        return _mm_blend_epi16(min(lhs.v, rhs.v), max(lhs.v, rhs.v), 0b11'11'00'00);
    }

    [[nodiscard]] friend iaarect operator+(iaarect const &lhs, ivec const &rhs) noexcept {
        return static_cast<__m128i>(lhs.v + rhs.xyxy());
    }

    [[nodiscard]] friend iaarect operator-(iaarect const &lhs, ivec const &rhs) noexcept {
        return static_cast<__m128i>(lhs.v - rhs.xyxy());
    }

    /** Expand the rectangle for the same amount in all directions.
     * @param lhs The original rectangle.
     * @param rhs How much should be added on each side of the rectangle,
     *            this value may be zero or negative.
     * @return A new rectangle expanded on each side.
     */
    template<typename T, std::enable_if_t<std::is_integral_v<T>,int> = 0>
    [[nodiscard]] friend iaarect expand(iaarect const &lhs, T rhs) noexcept {
        ttlet _0000 = _mm_setzero_si128();
        ttlet _000r = _mm_insert_epi32(_0000, rhs, 0);
        ttlet _00rr = ivec{_mm_shuffle_epi32(_000r, _MM_SHUFFLE(1,1,0,0))};
        ttlet _rr00 = ivec{_mm_shuffle_epi32(_000r, _MM_SHUFFLE(0,0,1,1))};
        return static_cast<__m128i>((lhs.v - _00rr) + _rr00);
    }
};

}

