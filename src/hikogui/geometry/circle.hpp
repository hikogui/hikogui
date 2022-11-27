// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file geometry/circle.hpp Defined the circle type.
 */

#pragma once

#include "point.hpp"
#include "axis_aligned_rectangle.hpp"
#include "quad.hpp"

namespace hi {
inline namespace v1 {

/** A type defining a 2D circle.
 */
class circle {
public:
    constexpr circle(circle const &other) noexcept = default;
    constexpr circle(circle &&other) noexcept = default;
    constexpr circle &operator=(circle const &other) noexcept = default;
    constexpr circle &operator=(circle &&other) noexcept = default;

    constexpr circle() noexcept : _v()
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit circle(f32x4 v) noexcept : _v(v)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr circle(point3 point, float radius) noexcept :
        _v(f32x4{point})
    {
        _v.w() = radius;
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr circle(aarectangle square) noexcept
    {
        hilet square_ = f32x4{square};

        // center=(p3 + p0)/2, radius=(p3 - p0)/2
        _v = (addsub<0b0011>(square_.zwzw(), square_.xyxy()) * 0.5f).xy0w();
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _v.w() == 0.0f;
    }

    [[nodiscard]] explicit operator bool () const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr float radius() const noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr point3 center() const noexcept
    {
        return point3{_v.xyz1()};
    }

    [[nodiscard]] constexpr friend circle operator+(circle const &lhs, float rhs) noexcept
    {
        return circle{lhs._v + insert<3>(f32x4{}, rhs)};
    }

    [[nodiscard]] constexpr friend circle operator-(circle const &lhs, float rhs) noexcept
    {
        return circle{lhs._v - insert<3>(f32x4{}, rhs)};
    }

    [[nodiscard]] constexpr friend circle operator*(circle const &lhs, float rhs) noexcept
    {
        return circle{lhs._v * insert<3>(f32x4::broadcast(1.0f), rhs)};
    }

    [[nodiscard]] constexpr friend point3 midpoint(circle const &rhs) noexcept
    {
        return point3{rhs.center()};
    }

    [[nodiscard]] constexpr friend aarectangle bounding_rectangle(circle const &rhs) noexcept
    {
        hilet p = rhs._v.xyxy();
        hilet r = neg<0b0011>(rhs._v.wwww());
        return aarectangle{p + r};
    }

private:
    // Stored as a center point (x, y, z), and radius (w).
    f32x4 _v;

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() >= 0.0f;
    }
};


}}

