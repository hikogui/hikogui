// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#pragma once

#include "vector3.hpp"
#include "point3.hpp"
#include "extent3.hpp"
#include "aarectangle.hpp"
#include "rectangle.hpp"
#include "../macros.hpp"
#include <tuple>
#include <exception>
#include <compare>

hi_export_module(hikogui.geometry : quad);

hi_export namespace hi::inline v1 {

class quad {
public:
    point3 p0; ///< Left-bottom
    point3 p1; ///< Right-bottom
    point3 p2; ///< Left-top
    point3 p3; ///< Right-top

    constexpr quad() noexcept : p0(), p1(), p2(), p3() {}

    constexpr quad(point3 p0, point3 p1, point3 p2, point3 p3) noexcept : p0(p0), p1(p1), p2(p2), p3(p3) {}

    constexpr quad(quad const &) noexcept = default;
    constexpr quad(quad &&) noexcept = default;
    constexpr quad &operator=(quad const &) noexcept = default;
    constexpr quad &operator=(quad &&) noexcept = default;

    constexpr quad(aarectangle const &rhs) noexcept
    {
        hilet tmp = f32x4{rhs};
        p0 = point3{tmp.xy01()};
        p1 = point3{tmp.zy01()};
        p2 = point3{tmp.xw01()};
        p3 = point3{tmp.zw01()};
    }

    constexpr quad(rectangle const &rhs) noexcept : p0(get<0>(rhs)), p1(get<1>(rhs)), p2(get<2>(rhs)), p3(get<3>(rhs)) {}

    /** The vector from left-bottom to right-bottom.
     */
    [[nodiscard]] constexpr vector3 bottom() const noexcept
    {
        return p1 - p0;
    }

    /** The vector from left-top to right-top.
     */
    [[nodiscard]] constexpr vector3 top() const noexcept
    {
        return p3 - p2;
    }

    /** The vector from left-bottom to left-top.
     */
    [[nodiscard]] constexpr vector3 left() const noexcept
    {
        return p2 - p0;
    }

    /** The vector from right-bottom to right-top.
     */
    [[nodiscard]] constexpr vector3 right() const noexcept
    {
        return p3 - p1;
    }

    /** Return the length of each edge.
     *
     * @return {bottom, left, top, right}.
     */
    [[nodiscard]] constexpr f32x4 edge_hypots() const noexcept
    {
        hilet[x, y, z, zeros] = transpose(f32x4{bottom()}, f32x4{left()}, f32x4{top()}, f32x4{right()});
        return sqrt(x * x + y * y + z * z);
    }

    [[nodiscard]] constexpr point3 &operator[](std::size_t index) noexcept
    {
        switch (index) {
        case 0: return p0;
        case 1: return p1;
        case 2: return p2;
        case 3: return p3;
        default: hi_no_default();
        }
    }

    [[nodiscard]] constexpr point3 const &operator[](std::size_t index) const noexcept
    {
        switch (index) {
        case 0: return p0;
        case 1: return p1;
        case 2: return p2;
        case 3: return p3;
        default: hi_no_default();
        }
    }

    template<std::size_t I>
    [[nodiscard]] constexpr friend point3 const &get(quad const &rhs) noexcept
    {
        static_assert(I < 4, "Index out of range.");

        if constexpr (I == 0) {
            return rhs.p0;
        } else if constexpr (I == 1) {
            return rhs.p1;
        } else if constexpr (I == 2) {
            return rhs.p2;
        } else {
            return rhs.p3;
        }
    }

    template<std::size_t I>
    [[nodiscard]] constexpr friend point3 &get(quad &rhs) noexcept
    {
        static_assert(I < 4, "Index out of range.");

        if constexpr (I == 0) {
            return rhs.p0;
        } else if constexpr (I == 1) {
            return rhs.p1;
        } else if constexpr (I == 2) {
            return rhs.p2;
        } else {
            return rhs.p3;
        }
    }

    /** Add a border around the quad.
     *
     * Move each corner of the quad by the given size outward in the direction of the edges.
     *
     * @param lhs A quad.
     * @param rhs The width and height to add to each corner of the quad, only (x, y) are used.
     * @return The new quad extended by the size and the new edge-lengths.
     */
    [[nodiscard]] friend constexpr std::pair<quad, f32x4> expand_and_edge_hypots(quad const &lhs, f32x4 const &rhs) noexcept
    {
        hilet t = f32x4{lhs.top()};
        hilet l = f32x4{lhs.left()};
        hilet b = f32x4{lhs.bottom()};
        hilet r = f32x4{lhs.right()};

        hilet[x, y, z, ones] = transpose(t, l, b, r);
        hilet square_lengths = x * x + y * y + z * z;
        hilet inv_lengths = rcp_sqrt(square_lengths);
        hilet norm_t = t * inv_lengths.xxxx();
        hilet norm_l = l * inv_lengths.yyyy();
        hilet norm_b = b * inv_lengths.zzzz();
        hilet norm_r = r * inv_lengths.wwww();

        hilet extra_width = rhs.xxxx();
        hilet extra_height = rhs.yyyy();

        hilet top_extra = vector3{norm_t * extra_width};
        hilet left_extra = vector3{norm_l * extra_height};
        hilet bottom_extra = vector3{norm_b * extra_width};
        hilet right_extra = vector3{norm_r * extra_height};

        hilet lengths = rcp(inv_lengths);

        hilet rhs_times_2 = rhs + rhs;

        return {
            quad{
                lhs.p0 - bottom_extra - left_extra,
                lhs.p1 + bottom_extra - right_extra,
                lhs.p2 - top_extra + left_extra,
                lhs.p3 + top_extra + right_extra},
            lengths + rhs_times_2.xyxy()};
    }

    /** Add a border around the quad.
     *
     * Move each corner of the quad by the given size outward in the direction of the edges.
     *
     * @param lhs A quad.
     * @param rhs The width and height to add to each corner of the quad.
     * @return The new quad extended by the size and the new edge-lengths.
     */
    [[nodiscard]] friend constexpr std::pair<quad, f32x4> expand_and_edge_hypots(quad const &lhs, extent2 const &rhs) noexcept
    {
        return expand_and_edge_hypots(lhs, f32x4{rhs});
    }

    /** Subtract a border from the quad.
     *
     * Move each corner of the quad by the given size inward in the direction of the edges.
     *
     * @param lhs A quad.
     * @param rhs The width and height to subtract from each corner of the quad.
     * @return The new quad shrunk by the size and the new edge-lengths.
     */
    [[nodiscard]] friend constexpr std::pair<quad, f32x4> shrink_and_edge_hypots(quad const &lhs, extent2 const &rhs) noexcept
    {
        return expand_and_edge_hypots(lhs, -f32x4{rhs});
    }

    /** Add a border around the quad.
     *
     * Move each corner of the quad by the given size outward in the direction of the edges.
     *
     * @param lhs A quad.
     * @param rhs The width and height to add to each corner of the quad.
     * @return The new quad extended by the size.
     */
    [[nodiscard]] friend constexpr quad operator+(quad const &lhs, extent2 const &rhs) noexcept
    {
        hilet[expanded_quad, new_lengths] = expand_and_edge_hypots(lhs, rhs);
        return expanded_quad;
    }

    [[nodiscard]] friend constexpr quad operator+(quad const &lhs, float rhs) noexcept
    {
        return lhs + extent2{rhs, rhs};
    }

    /** Add a border around the quad.
     *
     * Move each corner of the quad by the given size outward in the direction of the edges.
     *
     * @param lhs A quad.
     * @param rhs The width and height to add to each corner of the quad.
     * @return The new quad extended by the size.
     */
    [[nodiscard]] friend constexpr quad operator-(quad const &lhs, extent2 const &rhs) noexcept
    {
        hilet[expanded_quad, new_lengths] = shrink_and_edge_hypots(lhs, rhs);
        return expanded_quad;
    }

    [[nodiscard]] friend constexpr quad operator-(quad const &lhs, float rhs) noexcept
    {
        return lhs - extent2{rhs, rhs};
    }

    [[nodiscard]] friend constexpr aarectangle bounding_rectangle(quad const &rhs) noexcept
    {
        auto min_p = rhs.p0;
        auto max_p = rhs.p0;

        min_p = min(min_p, rhs.p1);
        max_p = max(max_p, rhs.p1);
        min_p = min(min_p, rhs.p2);
        max_p = max(max_p, rhs.p2);
        min_p = min(min_p, rhs.p3);
        max_p = max(max_p, rhs.p3);
        return aarectangle{point2{min_p}, point2{max_p}};
    }

    constexpr quad &operator+=(extent2 const &rhs) noexcept
    {
        return *this = *this + rhs;
    }

    [[nodiscard]] friend constexpr bool operator==(quad const &lhs, quad const &rhs) noexcept = default;
};

} // namespace hi::inline v1
