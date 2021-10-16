// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../rapid/numeric_array.hpp"
#include "axis_aligned_rectangle.hpp"
#include "../alignment.hpp"
#include <array>

namespace tt {

/** A rectangle / parallelogram in 3D space.
 *
 * This class actually describes a parallelogram in 3D space using
 * a point to the left bottom corner and a right & up vector.
 *
 */
class rectangle {
public:
    point3 origin;
    vector3 right;
    vector3 up;

    constexpr rectangle() noexcept : origin(), right(), up() {}
    constexpr rectangle(rectangle const &rhs) noexcept = default;
    constexpr rectangle &operator=(rectangle const &rhs) noexcept = default;
    constexpr rectangle(rectangle &&rhs) noexcept = default;
    constexpr rectangle &operator=(rectangle &&rhs) noexcept = default;

    /** Create a rectangle from a corner point and two vectors.
     *
     * @param origin The left-bottom corner.
     * @param right The right vector.
     * @param up The up vector.
     */
    constexpr rectangle(point3 origin, vector3 right, vector3 up) noexcept :
        origin(origin), right(right), up(up) {}

    /** Create a rectangle from 4 corner points.
     *
     * @param origin The left-bottom corner.
     * @param right_bottom The right-bottom corner.
     * @param left_top The left-top corner.
     * @param right_top The right-top corner.
     */
    constexpr rectangle(point3 origin, point3 right_bottom, point3 left_top, point3 right_top) noexcept :
        rectangle(origin, right_bottom - origin, left_top - origin) {}

    constexpr rectangle(aarectangle rhs) noexcept
    {
        ttlet p0 = get<0>(rhs);
        ttlet p2 = get<2>(rhs);
        ttlet diagonal = static_cast<f32x4>(p2 - p0);

        origin = p0;
        right = vector3{diagonal.x000()};
        up = vector3{diagonal._0y00()};
    }

    constexpr rectangle &operator=(aarectangle rhs) noexcept
    {
        ttlet p0 = get<0>(rhs);
        ttlet p2 = get<2>(rhs);
        ttlet diagonal = static_cast<f32x4>(p2 - p0);

        origin = p0;
        right = vector3{diagonal.x000()};
        up = vector3{diagonal._0y00()};
        return *this;
    }

    constexpr rectangle(point3 origin, extent2 extent) noexcept :
        corners(origin, extent.right(), extent.up())
    {
    }

    /** Check if the rectangle has an area.
     *
     * @return True is there is a area.
     */
    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        // min() is smallest normal float.
        return area() > std::numeric_limits<float>::min();
    }

    /** Check if this is a rectangle.
     *
     * @return True if rectangle, false if another parallelogram.
     */
    [[nodiscard]] constexpr bool is_rectangle() const noexcept
    {
        ttlet dp = dot(right, up);
        return -std::numeric_limits<float>::min() <= dp and dp <= std::numeric_limits<float>::min();
    }

    /** Check if this is an axis aligned rectangle.
     *
     * @return True if this is a axis aligned rectangle.
     */
    [[nodiscard]] constexpr bool is_axis_aligned() const noexcept
    {
        ttlet should_be_zeroes = static_cast<f32x4>(right).yz00() | static_cast<f32x4>(up)._00xz();
        return not should_be_zeroes;
    }

    /** The axis-aligned bounding box around the rectangle.
     */
    [[nodiscard]] constexpr aarectangle bounding_box() const noexcept
    {
        auto left_bottom = f32x4::broadcast(std::numeric_limits<float>::max());
        auto right_top = f32x4::broadcast(-std::numeric_limits<float>::max());

        ttlet p0 = origin;
        left_bottom = min(left_bottom, static_cast<f32x4>(p0));
        right_top = max(right_top, static_cast<f32x4>(p0));

        ttlet p1 = p0 + right;
        left_bottom = min(left_bottom, static_cast<f32x4>(p1));
        right_top = max(right_top, static_cast<f32x4>(p1));

        ttlet p2 = p0 + up;
        left_bottom = min(left_bottom, static_cast<f32x4>(p2));
        right_top = max(right_top, static_cast<f32x4>(p2));

        ttlet p3 = p2 + right;
        left_bottom = min(left_bottom, static_cast<f32x4>(p3));
        right_top = max(right_top, static_cast<f32x4>(p3));

        return aarectangle{left_bottom.xy00() | right_top._00xy()};
    }

    /** The width, or length of the right vector.
     */
    [[nodiscard]] constexpr float width() const noexcept
    {
        return hypot(right);
    }

    /** The height, or length of the up vector.
     */
    [[nodiscard]] constexpr float height() const noexcept
    {
        return hypot(up);
    }

    /** The size, or length of the right and up vectors.
     */
    [[nodiscard]] constexpr extent2 size() const noexcept
    {
        return {width(), height()};
    }

    [[nodiscard]] constexpr float area() const noexcept
    {
        return hypot(cross(right, up));
    }

    [[nodiscard]] constexpr point3 operator[](size_t i) const noexcept
    {
        switch (i) {
        case 0: return get<0>(*this);
        case 1: return get<1>(*this);
        case 2: return get<2>(*this);
        case 3: return get<3>(*this);
        default: tt_no_default();
        }
    }

    template<size_t I>
    [[nodiscard]] friend constexpr point3 get(rectangle const &rhs) noexcept
    {
        static_assert(I < 4);
        if constexpr (I == 0) {
            return origin;
        } else if constexpr (I == 1) {
            return origin + right;
        } else if constexpr (I == 2) {
            return origin + up;
        } else {
            return origin + right + up;
        }
    }

    /** Expand the rectangle by adding an absolute distance on each side.
     *
     * The expansion is done by:
     *  - translating the origin point in opposite direction of the
     *    right and up vectors.
     *  - increase the size of the right and up vectors twice.
     *
     * It is possible for the rectangle to flip, when the right-hand-side is negative.
     *
     * @param lhs The rectangle to expand.
     * @param rhs The scalar value which is added to the rectangle in each side.
     * @return A new rectangle expanded in each side.
     */
    [[nodiscard]] friend constexpr rectangle expand(rectangle const &lhs, float rhs) noexcept
    {
        ttlet extra_right = normalize(right) * rhs;
        ttlet extra_up = normalize(up) * rhs;
        ttlet extra_diagonal = extraright + extraup;

        return {
            origin - extra_diagonal
            right + 2.0f * extra_right,
            up + 2.0f * extra_up
        };
    }

    /** Shrink the rectangle by removing an absolute distance on each side.
     *
     * The shrinking is done by:
     *  - translating the origin point in the same direction of the
     *    right and up vectors.
     *  - decreasing the size of the right and up vectors twice.
     *
     * It is possible for the rectangle to flip, when the right-hand-side is positive.
     *
     * @param lhs The rectangle to shrink.
     * @param rhs The scalar value which is removed from the rectangle in each side.
     * @return A new rectangle shrunk in each side.
     */
    [[nodiscard]] friend constexpr rectangle shrink(rectangle const &lhs, float rhs) noexcept
    {
        return expand(lhs, -rhs);
    }


};

} // namespace tt
