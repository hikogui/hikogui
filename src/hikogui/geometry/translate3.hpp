// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version float{1}.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix2.hpp"
#include "identity.hpp"
#include "rotate3.hpp"
#include "vector2.hpp"
#include "vector3.hpp"
#include "point2.hpp"
#include "point3.hpp"
#include "translate2.hpp"
#include <concepts>

namespace hi { inline namespace v1 {

class translate3;
[[nodiscard]] constexpr point3 operator*(translate3 const& lhs, point3 const& rhs) noexcept;


class translate3 {
public:
    using array_type = simd<float, 4>;
    using value_type = array_type::value_type;

    constexpr translate3(translate3 const&) noexcept = default;
    constexpr translate3(translate3&&) noexcept = default;
    constexpr translate3& operator=(translate3 const&) noexcept = default;
    constexpr translate3& operator=(translate3&&) noexcept = default;

    [[nodiscard]] constexpr operator matrix3() const noexcept
    {
        hi_axiom(holds_invariant());
        hilet ones = array_type::broadcast(1.0f);
        return matrix3{ones.x000(), ones._0y00(), ones._00z0(), ones._000w() + _v};
    }

    [[nodiscard]] constexpr translate3() noexcept : _v(0.0f, 0.0f, 0.0f, 0.0f) {}

    [[nodiscard]] constexpr translate3(geo::identity const&) noexcept : translate3() {}

    [[nodiscard]] constexpr explicit operator array_type() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr explicit translate3(array_type const& other) noexcept : _v(other)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit translate3(aarectangle const& other) noexcept :
        _v(static_cast<array_type>(get<0>(other)).xy00())
    {
    }

    [[nodiscard]] constexpr explicit translate3(aarectangle const& other, float z) noexcept
        : _v(static_cast<array_type>(get<0>(other)).xy00())
    {
        _v.z() = z;
    }

    [[nodiscard]] constexpr translate3(translate2 const& other) noexcept : _v(static_cast<array_type>(other))
    {
    }

    [[nodiscard]] constexpr translate3(translate2 const& other, float z) noexcept : _v(static_cast<array_type>(other))
    {
        _v.z() = z;
    }

    [[nodiscard]] constexpr explicit operator translate2() const noexcept
    {
        auto tmp = _v;
        tmp.z() = 0.0f;
        return translate2{tmp};
    }

    [[nodiscard]] constexpr explicit translate3(vector3 const& other) noexcept
        : _v(static_cast<array_type>(other))
    {
    }

    [[nodiscard]] constexpr explicit translate3(point3 const& other) noexcept
        : _v(static_cast<array_type>(other).xyz0())
    {
    }

    [[nodiscard]] constexpr translate3(float x, float y, float z = 0.0f) noexcept
        : _v(x, y, z, 0.0f)
    {
    }

    [[nodiscard]] constexpr float x() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float y() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float z() const noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float& x() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float& y() noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float& z() noexcept
    {
        return _v.z();
    }

    /** Align a rectangle within another rectangle.
     * @param src_rectangle The rectangle to translate into the dst_rectangle
     * @param dst_rectangle The destination rectangle.
     * @param alignment How the source rectangle should be aligned inside the destination rectangle.
     * @return Translation to move the src_rectangle into the dst_rectangle.
     */
    [[nodiscard]] constexpr static translate3 align(
        aarectangle src_rectangle,
        aarectangle dst_rectangle,
        alignment alignment) noexcept
    {
        auto x = float{0};
        if (alignment == horizontal_alignment::left) {
            x = dst_rectangle.left();

        } else if (alignment == horizontal_alignment::right) {
            x = dst_rectangle.right() - src_rectangle.width();

        } else if (alignment == horizontal_alignment::center) {
            x = dst_rectangle.center() - src_rectangle.width() * 0.5f;

        } else {
            hi_no_default();
        }

        auto y = float{0};
        if (alignment == vertical_alignment::bottom) {
            y = dst_rectangle.bottom();

        } else if (alignment == vertical_alignment::top) {
            y = dst_rectangle.top() - src_rectangle.height();

        } else if (alignment == vertical_alignment::middle) {
            y = dst_rectangle.middle() - src_rectangle.height() * 0.5f;

        } else {
            hi_no_default();
        }

        return translate3{x - src_rectangle.left(), y - src_rectangle.bottom()};
    }

    [[nodiscard]] constexpr vector2 operator*(vector2 const& rhs) const noexcept
    {
        return rhs;
    }

    [[nodiscard]] constexpr friend translate3 operator*(translate3 const& lhs, geo::identity const&) noexcept
    {
        return lhs;
    }

    [[nodiscard]] constexpr friend matrix3 operator*(translate3 const& lhs, matrix3 const& rhs) noexcept
    {
        return matrix3{get<0>(rhs), get<1>(rhs), get<2>(rhs), get<3>(rhs) + lhs._v};
    }

    [[nodiscard]] constexpr friend matrix3 operator*(translate3 const& lhs, rotate3 const& rhs) noexcept
    {
        return lhs * matrix3(rhs);
    }

    [[nodiscard]] constexpr friend translate3 operator*(translate3 const &lhs, translate3 const& rhs) noexcept
    {
        return translate3{lhs._v + rhs._v};
    }

    [[nodiscard]] constexpr friend rectangle operator*(translate3 const& lhs, aarectangle const& rhs) noexcept
    {
        hilet rhs_ = rectangle{rhs};
        return rectangle{lhs * rhs_.origin, rhs_.right, rhs_.up};
    }

    [[nodiscard]] constexpr friend bool operator==(translate3 const& lhs, translate3 const& rhs) noexcept
    {
        return equal(lhs._v, rhs._v);
    }

    [[nodiscard]] constexpr translate3 operator~() const noexcept
    {
        return translate3{-_v};
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() == 0.0f;
    }

    [[nodiscard]] friend constexpr translate3 round(translate3 const& rhs) noexcept
    {
        return translate3{round(rhs._v)};
    }

private:
    array_type _v;
};

[[nodiscard]] constexpr translate3 translate_z(float z) noexcept
{
    return translate3{0.0f, 0.0f, z};
}

[[nodiscard]] constexpr vector3 operator*(translate3 const &lhs, vector3 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr point3 operator*(translate3 const& lhs, point3 const& rhs) noexcept
{
    return point3{f32x4{lhs} + f32x4{rhs}};
}

constexpr point3& operator*=(point3& lhs, translate3 const& rhs) noexcept
{
    return lhs = rhs * lhs;
}

[[nodiscard]] constexpr rectangle operator*(translate3 const& lhs, rectangle const& rhs) noexcept
{
    return rectangle{lhs * rhs.origin, rhs.right, rhs.up};
}

[[nodiscard]] constexpr quad operator*(translate3 const& lhs, quad const& rhs) noexcept
{
    return quad{lhs * rhs.p0, lhs * rhs.p1, lhs * rhs.p2, lhs * rhs.p3};
}

[[nodiscard]] constexpr circle operator*(translate3 const& lhs, circle const& rhs) noexcept
{
    return circle{f32x4{rhs} + f32x4{lhs}};
}

[[nodiscard]] constexpr line_segment operator*(translate3 const& lhs, line_segment const& rhs) noexcept
{
    return line_segment{lhs * rhs.origin(), rhs.direction()};
}

}} // namespace hi::inline v1
