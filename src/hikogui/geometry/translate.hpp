// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix.hpp"
#include "identity.hpp"
#include "rotate.hpp"

namespace hi::inline v1 {
namespace geo {

template<int D>
class translate {
public:
    static_assert(D == 2 || D == 3, "Only 2D or 3D translation-matrices are supported");

    constexpr translate(translate const &) noexcept = default;
    constexpr translate(translate &&) noexcept = default;
    constexpr translate &operator=(translate const &) noexcept = default;
    constexpr translate &operator=(translate &&) noexcept = default;

    [[nodiscard]] constexpr operator matrix<2>() const noexcept requires(D == 2)
    {
        hi_axiom(holds_invariant());
        hilet ones = f32x4::broadcast(1.0);
        return matrix<2>{ones.x000(), ones._0y00(), ones._00z0(), ones._000w() + _v};
    }

    [[nodiscard]] constexpr operator matrix<3>() const noexcept
    {
        hi_axiom(holds_invariant());
        hilet ones = f32x4::broadcast(1.0);
        return matrix<3>{ones.x000(), ones._0y00(), ones._00z0(), ones._000w() + _v};
    }

    [[nodiscard]] constexpr translate() noexcept : _v() {}

    [[nodiscard]] constexpr translate(identity const &) noexcept : translate() {}

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        hi_axiom(holds_invariant());
        return _v;
    }

    [[nodiscard]] constexpr explicit translate(f32x4 const &other) noexcept : _v(other)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit translate(aarectangle const &other) noexcept : _v(static_cast<f32x4>(get<0>(other)).xy00())
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit translate(aarectangle const &other, float z) noexcept requires(D == 3) :
        _v(static_cast<f32x4>(get<0>(other)).xy00())
    {
        _v.z() = z;
        hi_axiom(holds_invariant());
    }

    template<int E>
    requires(E < D) [[nodiscard]] constexpr translate(translate<E> const &other) noexcept : _v(static_cast<f32x4>(other))
    {
        hi_axiom(holds_invariant());
    }

    template<int E>
    requires(E <= D) [[nodiscard]] constexpr explicit translate(vector<E> const &other) noexcept : _v(static_cast<f32x4>(other))
    {
        hi_axiom(holds_invariant());
    }

    template<int E>
    requires(E <= D) [[nodiscard]] constexpr explicit translate(point<E> const &other) noexcept :
        _v(static_cast<f32x4>(other).xyz0())
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr translate(float x, float y) noexcept requires(D == 2) : _v(x, y, 0.0, 0.0) {}

    [[nodiscard]] constexpr translate(float x, float y, float z = 0.0) noexcept requires(D == 3) : _v(x, y, z, 0.0) {}

    /** Align a rectangle within another rectangle.
     * @param src_rectangle The rectangle to translate into the dst_rectangle
     * @param dst_rectangle The destination rectangle.
     * @param alignment How the source rectangle should be aligned inside the destination rectangle.
     * @return Translation to move the src_rectangle into the dst_rectangle.
     */
    [[nodiscard]] constexpr static translate
    align(aarectangle src_rectangle, aarectangle dst_rectangle, alignment alignment) noexcept
    {
        auto x = 0.0f;
        if (alignment == horizontal_alignment::left) {
            x = dst_rectangle.left();

        } else if (alignment == horizontal_alignment::right) {
            x = dst_rectangle.right() - src_rectangle.width();

        } else if (alignment == horizontal_alignment::center) {
            x = dst_rectangle.center() - src_rectangle.width() * 0.5f;

        } else {
            hi_no_default();
        }

        auto y = 0.0f;
        if (alignment == vertical_alignment::bottom) {
            y = dst_rectangle.bottom();

        } else if (alignment == vertical_alignment::top) {
            y = dst_rectangle.top() - src_rectangle.height();

        } else if (alignment == vertical_alignment::middle) {
            y = dst_rectangle.middle() - src_rectangle.height() * 0.5f;

        } else {
            hi_no_default();
        }

        return translate{x - src_rectangle.left(), y - src_rectangle.bottom()};
    }

    template<int E>
    [[nodiscard]] constexpr vector<E> operator*(vector<E> const &rhs) const noexcept
    {
        // Vectors are not translated.
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return rhs;
    }

    template<int E>
    [[nodiscard]] constexpr point<std::max(D, E)> operator*(point<E> const &rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return point<std::max(D, E)>{_v + static_cast<f32x4>(rhs)};
    }

    [[nodiscard]] constexpr aarectangle operator*(aarectangle const &rhs) const noexcept requires(D == 2)
    {
        return aarectangle{*this * get<0>(rhs), *this * get<3>(rhs)};
    }

    [[nodiscard]] constexpr rectangle operator*(aarectangle const &rhs) const noexcept requires(D == 3)
    {
        return *this * rectangle{rhs};
    }

    [[nodiscard]] constexpr rectangle operator*(rectangle const &rhs) const noexcept
    {
        return rectangle{*this * rhs.origin, rhs.right, rhs.up};
    }

    [[nodiscard]] constexpr quad operator*(quad const &rhs) const noexcept
    {
        return quad{*this * rhs.p0, *this * rhs.p1, *this * rhs.p2, *this * rhs.p3};
    }

    [[nodiscard]] constexpr circle operator*(circle const &rhs) const noexcept
    {
        return circle{f32x4{rhs} + _v};
    }

    [[nodiscard]] constexpr line_segment operator*(line_segment const &rhs) const noexcept
    {
        return line_segment{*this * rhs.origin(), rhs.direction()};
    }

    [[nodiscard]] constexpr translate operator*(identity const &) const noexcept
    {
        hi_axiom(holds_invariant());
        return *this;
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(matrix<E> const &rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return matrix<std::max(D, E)>{get<0>(rhs), get<1>(rhs), get<2>(rhs), get<3>(rhs) + _v};
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(rotate<E> const &rhs) const noexcept
    {
        return *this * matrix<E>(rhs);
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(translate<E> const &rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return translate<std::max(D, E)>{_v + static_cast<f32x4>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr bool operator==(translate<E> const &rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return _v == static_cast<f32x4>(rhs);
    }

    [[nodiscard]] constexpr translate operator~() const noexcept
    {
        return translate{-_v};
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() == 0.0f && (D == 3 || _v.z() == 0.0f);
    }

    [[nodiscard]] friend constexpr translate round(translate const &rhs) noexcept
    {
        return translate{round(rhs._v)};
    }

private:
    f32x4 _v;
};

} // namespace geo

using translate2 = geo::translate<2>;
using translate3 = geo::translate<3>;

constexpr translate3 translate_z(float z) noexcept
{
    return translate3{0.0f, 0.0f, z};
}

} // namespace hi::inline v1
