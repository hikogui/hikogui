// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix.hpp"
#include "identity.hpp"
#include "translate.hpp"
#include "extent.hpp"

namespace hi::inline v1 {
namespace geo {

template<int D>
class scale {
public:
    static_assert(D == 2 || D == 3, "Only 2D or 3D scale-matrices are supported");

    constexpr scale(scale const &) noexcept = default;
    constexpr scale(scale &&) noexcept = default;
    constexpr scale &operator=(scale const &) noexcept = default;
    constexpr scale &operator=(scale &&) noexcept = default;

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        hi_axiom(holds_invariant());
        return _v;
    }

    [[nodiscard]] constexpr explicit operator extent<2>() const noexcept requires(D == 2)
    {
        return extent<2>{_v.xy00()};
    }

    [[nodiscard]] constexpr explicit operator extent<3>() const noexcept requires(D == 3)
    {
        return extent<3>{_v.xyz0()};
    }

    [[nodiscard]] constexpr explicit scale(f32x4 const &v) noexcept : _v(v)
    {
        hi_axiom(holds_invariant());
    }

    template<int E>
    requires(E <= D) [[nodiscard]] constexpr explicit scale(vector<E> const &v) noexcept : _v(static_cast<f32x4>(v).xyz1())
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr operator matrix<D>() const noexcept
    {
        hi_axiom(holds_invariant());
        return matrix<D>{_v.x000(), _v._0y00(), _v._00z0(), _v._000w()};
    }

    [[nodiscard]] constexpr scale() noexcept : _v(1.0, 1.0, 1.0, 1.0) {}

    [[nodiscard]] constexpr scale(identity const &) noexcept : _v(1.0, 1.0, 1.0, 1.0) {}

    [[nodiscard]] constexpr scale(float value) noexcept requires(D == 2) : _v(value, value, 1.0, 1.0) {}

    [[nodiscard]] constexpr scale(float value) noexcept requires(D == 3) : _v(value, value, value, 1.0) {}

    [[nodiscard]] constexpr scale(float x, float y) noexcept requires(D == 2) : _v(x, y, 1.0, 1.0) {}

    [[nodiscard]] constexpr scale(float x, float y, float z = 1.0) noexcept requires(D == 3) : _v(x, y, z, 1.0) {}

    /** Get a uniform-scale-transform to scale an extent to another extent.
     * @param src_extent The extent to transform
     * @param dst_extent The extent to scale to.
     * @return a scale to transform the src_extent to the dst_extent.
     */
    template<int E, int F>
    requires(E <= D && F <= D) [[nodiscard]] static constexpr scale uniform(extent<E> src_extent, extent<F> dst_extent) noexcept
    {
        hi_axiom(
            dst_extent.width() != 0.0f && src_extent.width() != 0.0f && dst_extent.height() != 0.0f &&
            src_extent.height() != 0.0f);

        if constexpr (D == 2) {
            hilet non_uniform_scale = static_cast<f32x4>(dst_extent).xyxy() / static_cast<f32x4>(src_extent).xyxy();
            hilet uniform_scale = std::min(non_uniform_scale.x(), non_uniform_scale.y());
            return scale{uniform_scale};

        } else if constexpr (D == 3) {
            hi_axiom(dst_extent.z() != 0.0f && src_extent.z() != 0.0f);
            hilet non_uniform_scale = static_cast<f32x4>(dst_extent).xyzx() / static_cast<f32x4>(src_extent).xyzx();
            hilet uniform_scale = std::min({non_uniform_scale.x(), non_uniform_scale.y(), non_uniform_scale.z()});
            return scale{uniform_scale};

        } else {
            hi_static_no_default();
        }
    }

    template<int E>
    [[nodiscard]] constexpr vector<E> operator*(vector<E> const &rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return vector<E>{_v * static_cast<f32x4>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr extent<E> operator*(extent<E> const &rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return extent<E>{_v * static_cast<f32x4>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr point<E> operator*(point<E> const &rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return point<E>{_v * static_cast<f32x4>(rhs)};
    }

    /** Scale a rectangle around it's center.
     */
    [[nodiscard]] constexpr aarectangle operator*(aarectangle const &rhs) const noexcept requires(D == 2)
    {
        return aarectangle{*this * get<0>(rhs), *this * get<3>(rhs)};
    }

    [[nodiscard]] constexpr rectangle operator*(rectangle const &rhs) const noexcept
    {
        return rectangle{*this * get<0>(rhs), *this * get<1>(rhs), *this * get<2>(rhs), *this * get<3>(rhs)};
    }

    [[nodiscard]] constexpr quad operator*(quad const &rhs) const noexcept
    {
        return quad{*this * rhs.p0, *this * rhs.p1, *this * rhs.p2, *this * rhs.p3};
    }

    /** scale the quad.
     *
     * Each edge of the quad scaled.
     *
     * @param lhs A quad.
     * @param rhs The width and height to scale each edge with.
     * @return The new quad extended by the size.
     */
    [[nodiscard]] friend constexpr quad scale_from_center(quad const &lhs, scale const &rhs) noexcept requires(D == 2)
    {
        hilet top_extra = (lhs.top() * rhs._v.x() - lhs.top()) * 0.5f;
        hilet bottom_extra = (lhs.bottom() * rhs._v.x() - lhs.bottom()) * 0.5f;
        hilet left_extra = (lhs.left() * rhs._v.y() - lhs.left()) * 0.5f;
        hilet right_extra = (lhs.right() * rhs._v.y() - lhs.right()) * 0.5f;

        return {
            lhs.p0 - bottom_extra - left_extra,
            lhs.p1 + bottom_extra - right_extra,
            lhs.p2 - top_extra + left_extra,
            lhs.p3 + top_extra + right_extra};
    }

    [[nodiscard]] constexpr scale operator*(identity const &) const noexcept
    {
        hi_axiom(holds_invariant());
        return *this;
    }

    template<int E>
    [[nodiscard]] constexpr auto operator*(scale<E> const &rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return scale<std::max(D, E)>{_v * static_cast<f32x4>(rhs)};
    }

    template<int E>
    [[nodiscard]] constexpr bool operator==(scale<E> const &rhs) const noexcept
    {
        hi_axiom(holds_invariant() && rhs.holds_invariant());
        return _v == static_cast<f32x4>(rhs);
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        if constexpr (D == 3) {
            return _v.w() == 1.0f;
        } else {
            return _v.z() == 1.0f and _v.w() == 1.0f;
        }
    }

private:
    f32x4 _v;
};

[[nodiscard]] constexpr scale<2> operator/(extent<2> const &lhs, extent<2> const &rhs) noexcept
{
    hi_axiom(rhs._v.x() != 0.0f);
    hi_axiom(rhs._v.y() != 0.0f);
    return scale<2>{lhs._v.xy11() / rhs._v.xy11()};
}

[[nodiscard]] constexpr scale<3> operator/(extent<3> const &lhs, extent<3> const &rhs) noexcept
{
    hi_axiom(rhs._v.x() != 0.0f);
    hi_axiom(rhs._v.y() != 0.0f);
    hi_axiom(rhs._v.z() != 0.0f);
    return scale<3>{lhs._v.xyz1() / rhs._v.xyz1()};
}

template<int D>
[[nodiscard]] constexpr matrix<D>
matrix<D>::uniform(aarectangle src_rectangle, aarectangle dst_rectangle, alignment alignment) noexcept
{
    hilet scale = hi::geo::scale<D>::uniform(src_rectangle.size(), dst_rectangle.size());
    hilet scaled_rectangle = scale * src_rectangle;
    hilet translation = translate<D>::align(scaled_rectangle, dst_rectangle, alignment);
    return translation * scale;
}

} // namespace geo

using scale2 = geo::scale<2>;
using scale3 = geo::scale<3>;

} // namespace hi::inline v1
