// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version float{1}.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "translate2.hpp"
#include "point3.hpp"
#include "../macros.hpp"
#include <concepts>
#include <exception>
#include <compare>

hi_export_module(hikogui.geometry : translate3);

hi_export namespace hi { inline namespace v1 {

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

    [[nodiscard]] constexpr translate3() noexcept : _v(0.0f, 0.0f, 0.0f, 0.0f) {}

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

}} // namespace hi::inline v1
