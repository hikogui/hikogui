// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version value_type{1}.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix.hpp"
#include "identity.hpp"
#include "rotate.hpp"

namespace hi::inline v1 {
namespace geo {

template<typename T, int D>
class translate {
public:
    static_assert(D == 2 || D == 3, "Only 2D or 3D translation-matrices are supported");

    using value_type = T;
    using array_type = simd<value_type, 4>;

    constexpr translate(translate const&) noexcept = default;
    constexpr translate(translate&&) noexcept = default;
    constexpr translate& operator=(translate const&) noexcept = default;
    constexpr translate& operator=(translate&&) noexcept = default;

    [[nodiscard]] constexpr operator matrix<2>() const noexcept
        requires std::is_same_v<value_type, float> and (D == 2)
    {
        hi_axiom(holds_invariant());
        hilet ones = array_type::broadcast(value_type{1});
        return matrix<2>{ones.x000(), ones._0y00(), ones._00z0(), ones._000w() + _v};
    }

    [[nodiscard]] constexpr operator matrix<3>() const noexcept
        requires std::is_same_v<value_type, float>
    {
        hi_axiom(holds_invariant());
        hilet ones = array_type::broadcast(value_type{1});
        return matrix<3>{ones.x000(), ones._0y00(), ones._00z0(), ones._000w() + _v};
    }

    [[nodiscard]] constexpr translate() noexcept : _v() {}

    [[nodiscard]] constexpr translate(identity const&) noexcept : translate() {}

    [[nodiscard]] constexpr explicit operator array_type() const noexcept
    {
        hi_axiom(holds_invariant());
        return _v;
    }

    [[nodiscard]] constexpr explicit translate(array_type const& other) noexcept : _v(other)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit translate(axis_aligned_rectangle<value_type> const& other) noexcept :
        _v(static_cast<array_type>(get<0>(other)).xy00())
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit translate(axis_aligned_rectangle<value_type> const& other, value_type z) noexcept
        requires(D == 3)
        : _v(static_cast<array_type>(get<0>(other)).xy00())
    {
        _v.z() = z;
        hi_axiom(holds_invariant());
    }

    template<int E>
        requires(E < D)
    [[nodiscard]] constexpr translate(translate<value_type, E> const& other) noexcept : _v(static_cast<array_type>(other))
    {
        hi_axiom(holds_invariant());
    }

    template<int E>
        requires(E < D)
    [[nodiscard]] constexpr translate(translate<value_type, E> const& other, value_type z) noexcept :
        _v(static_cast<array_type>(other))
    {
        _v.z() = z;
        hi_axiom(holds_invariant());
    }

    template<int E>
        requires(E <= D)
    [[nodiscard]] constexpr explicit translate(vector<value_type, E> const& other) noexcept : _v(static_cast<array_type>(other))
    {
        hi_axiom(holds_invariant());
    }

    template<int E>
        requires(E <= D)
    [[nodiscard]] constexpr explicit translate(point<value_type, E> const& other) noexcept :
        _v(static_cast<array_type>(other).xyz0())
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr translate(value_type x, value_type y) noexcept
        requires(D == 2)
        : _v(x, y, value_type{0}, value_type{0})
    {
    }

    [[nodiscard]] constexpr translate(value_type x, value_type y, value_type z = value_type{0}) noexcept
        requires(D == 3)
        : _v(x, y, z, value_type{0})
    {
    }

    [[nodiscard]] constexpr value_type x() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr value_type y() const noexcept
        requires(D >= 2)
    {
        return _v.y();
    }

    [[nodiscard]] constexpr value_type z() const noexcept
        requires(D >= 3)
    {
        return _v.z();
    }

    [[nodiscard]] constexpr value_type& x() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr value_type& y() noexcept
        requires(D >= 2)
    {
        return _v.y();
    }

    [[nodiscard]] constexpr value_type& z() noexcept
        requires(D >= 3)
    {
        return _v.z();
    }

    /** Align a rectangle within another rectangle.
     * @param src_rectangle The rectangle to translate into the dst_rectangle
     * @param dst_rectangle The destination rectangle.
     * @param alignment How the source rectangle should be aligned inside the destination rectangle.
     * @return Translation to move the src_rectangle into the dst_rectangle.
     */
    [[nodiscard]] constexpr static translate align(
        axis_aligned_rectangle<value_type> src_rectangle,
        axis_aligned_rectangle<value_type> dst_rectangle,
        alignment alignment) noexcept
    {
        auto x = value_type{0};
        if (alignment == horizontal_alignment::left) {
            x = dst_rectangle.left();

        } else if (alignment == horizontal_alignment::right) {
            x = dst_rectangle.right() - src_rectangle.width();

        } else if (alignment == horizontal_alignment::center) {
            x = dst_rectangle.center() - src_rectangle.width() * 0.5f;

        } else {
            hi_no_default();
        }

        auto y = value_type{0};
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
    [[nodiscard]] constexpr vector<value_type, E> operator*(vector<value_type, E> const& rhs) const noexcept
    {
        // Vectors are not translated.
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return rhs;
    }

    template<int E>
    [[nodiscard]] constexpr point<value_type, std::max(D, E)> operator*(point<value_type, E> const& rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return point<value_type, std::max(D, E)>{_v + static_cast<array_type>(rhs)};
    }

    constexpr friend point<value_type, 2>& operator*=(point<value_type, 2>& lhs, translate const& rhs) noexcept
        requires(D == 2)
    {
        return lhs = rhs * lhs;
    }

    [[nodiscard]] constexpr axis_aligned_rectangle<value_type>
    operator*(axis_aligned_rectangle<value_type> const& rhs) const noexcept
        requires(D == 2)
    {
        return axis_aligned_rectangle<value_type>{*this * get<0>(rhs), *this * get<3>(rhs)};
    }

    constexpr friend axis_aligned_rectangle<value_type>&
    operator*=(axis_aligned_rectangle<value_type>& lhs, translate const& rhs) noexcept
        requires(D == 2)
    {
        return lhs = rhs * lhs;
    }

    [[nodiscard]] constexpr rectangle operator*(axis_aligned_rectangle<value_type> const& rhs) const noexcept
        requires std::is_same_v<value_type, float> and (D == 3)
    {
        return *this * rectangle{rhs};
    }

    [[nodiscard]] constexpr rectangle operator*(rectangle const& rhs) const noexcept
        requires std::is_same_v<value_type, float>
    {
        return rectangle{*this * rhs.origin, rhs.right, rhs.up};
    }

    [[nodiscard]] constexpr quad operator*(quad const& rhs) const noexcept
        requires std::is_same_v<value_type, float>
    {
        return quad{*this * rhs.p0, *this * rhs.p1, *this * rhs.p2, *this * rhs.p3};
    }

    [[nodiscard]] constexpr circle operator*(circle const& rhs) const noexcept
        requires std::is_same_v<value_type, float>
    {
        return circle{array_type{rhs} + _v};
    }

    [[nodiscard]] constexpr line_segment operator*(line_segment const& rhs) const noexcept
        requires std::is_same_v<value_type, float>
    {
        return line_segment{*this * rhs.origin(), rhs.direction()};
    }

    [[nodiscard]] constexpr translate operator*(identity const&) const noexcept
    {
        hi_axiom(holds_invariant());
        return *this;
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(matrix<E> const& rhs) const noexcept
        requires std::is_same_v<value_type, float>
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return matrix<std::max(D, E)>{get<0>(rhs), get<1>(rhs), get<2>(rhs), get<3>(rhs) + _v};
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(rotate<E> const& rhs) const noexcept
        requires std::is_same_v<value_type, float>
    {
        return *this * matrix<E>(rhs);
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(translate<value_type, E> const& rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return translate<value_type, std::max(D, E)>{_v + static_cast<array_type>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr bool operator==(translate<value_type, E> const& rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return equal(_v, static_cast<array_type>(rhs));
    }

    [[nodiscard]] constexpr translate operator~() const noexcept
    {
        return translate{-_v};
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() == value_type{0} && (D == 3 || _v.z() == value_type{0});
    }

    [[nodiscard]] friend constexpr translate round(translate const& rhs) noexcept
        requires std::is_same_v<value_type, float>
    {
        return translate{round(rhs._v)};
    }

private:
    array_type _v;
};

} // namespace geo

using translate2 = geo::translate<float, 2>;
using translate3 = geo::translate<float, 3>;
using translate2i = geo::translate<int, 2>;
using translate3i = geo::translate<int, 3>;

constexpr translate3 translate_z(float z) noexcept
{
    return translate3{float{0}, float{0}, z};
}

constexpr translate3i translate_z(int z) noexcept
{
    return translate3i{int{0}, int{0}, z};
}

template<>
[[nodiscard]] constexpr translate2 narrow_cast(translate2i const& rhs) noexcept
{
    return {narrow_cast<float>(rhs.x()), narrow_cast<float>(rhs.y())};
}

} // namespace hi::inline v1
