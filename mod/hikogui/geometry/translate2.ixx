// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version float{1}.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <concepts>
#include <exception>
#include <compare>

export module hikogui_geometry : translate2;
import : aarectangle;
import : point2;

export namespace hi { inline namespace v1 {

class translate2 {
public:
    using array_type = simd<float, 4>;
    using value_type = array_type::value_type;

    constexpr translate2(translate2 const&) noexcept = default;
    constexpr translate2(translate2&&) noexcept = default;
    constexpr translate2& operator=(translate2 const&) noexcept = default;
    constexpr translate2& operator=(translate2&&) noexcept = default;

    [[nodiscard]] constexpr translate2() noexcept : _v(0.0f, 0.0f, 0.0f, 0.0f) {}

    [[nodiscard]] constexpr explicit operator array_type() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr explicit translate2(array_type const& other) noexcept : _v(other)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit translate2(aarectangle const& other) noexcept :
        _v(static_cast<array_type>(get<0>(other)).xy00())
    {
    }

    [[nodiscard]] constexpr explicit translate2(aarectangle const& other, float z) noexcept :
        _v(static_cast<array_type>(get<0>(other)).xy00())
    {
        _v.z() = z;
    }

    [[nodiscard]] constexpr explicit translate2(vector2 const& other) noexcept : _v(static_cast<array_type>(other)) {}

    [[nodiscard]] constexpr explicit translate2(point2 const& other) noexcept : _v(static_cast<array_type>(other).xy00()) {}

    [[nodiscard]] constexpr translate2(float x, float y) noexcept : _v(x, y, 0.0f, 0.0f) {}

    [[nodiscard]] constexpr float x() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float y() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float& x() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float& y() noexcept
    {
        return _v.y();
    }

    /** Align a rectangle within another rectangle.
     * @param src_rectangle The rectangle to translate into the dst_rectangle
     * @param dst_rectangle The destination rectangle.
     * @param alignment How the source rectangle should be aligned inside the destination rectangle.
     * @return Translation to move the src_rectangle into the dst_rectangle.
     */
    [[nodiscard]] constexpr static translate2
    align(aarectangle src_rectangle, aarectangle dst_rectangle, alignment alignment) noexcept
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

        return translate2{x - src_rectangle.left(), y - src_rectangle.bottom()};
    }

    [[nodiscard]] constexpr friend bool operator==(translate2 const& lhs, translate2 const& rhs) noexcept
    {
        return equal(lhs._v, rhs._v);
    }

    [[nodiscard]] constexpr translate2 operator~() const noexcept
    {
        return translate2{-_v};
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.z() == 0.0f and _v.w() == 0.0f;
    }

    [[nodiscard]] friend constexpr translate2 round(translate2 const& rhs) noexcept
    {
        return translate2{round(rhs._v)};
    }

private:
    array_type _v;
};

}} // namespace hi::v1
