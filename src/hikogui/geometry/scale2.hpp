// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix2.hpp"
#include "identity.hpp"
#include "translate2.hpp"
#include "extent2.hpp"

namespace hi { inline namespace v1 {

class scale2 {
public:
    using array_type = simd<float, 4>;
    using value_type = array_type::value_type;

    constexpr scale2(scale2 const&) noexcept = default;
    constexpr scale2(scale2&&) noexcept = default;
    constexpr scale2& operator=(scale2 const&) noexcept = default;
    constexpr scale2& operator=(scale2&&) noexcept = default;

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr explicit scale2(f32x4 const& v) noexcept : _v(v)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit scale2(vector2 const& v) noexcept : _v(static_cast<f32x4>(v).xyz1()) {}

    [[nodiscard]] constexpr operator matrix2() const noexcept
    {
        return matrix2{_v.x000(), _v._0y00(), _v._00z0(), _v._000w()};
    }

    [[nodiscard]] constexpr scale2() noexcept : _v(1.0f, 1.0f, 1.0f, 1.0f) {}

    [[nodiscard]] constexpr scale2(geo::identity const&) noexcept : _v(1.0f, 1.0f, 1.0f, 1.0f) {}

    [[nodiscard]] constexpr scale2(float value) noexcept : _v(value, value, value, 1.0f) {}

    [[nodiscard]] constexpr scale2(float x, float y) noexcept : _v(x, y, 1.0f, 1.0f) {}

    /** Get a uniform-scale-transform to scale an extent to another extent.
     * @param src_extent The extent to transform
     * @param dst_extent The extent to scale to.
     * @return a scale to transform the src_extent to the dst_extent.
     */
    [[nodiscard]] static constexpr scale2 uniform(extent2 src_extent, extent2 dst_extent) noexcept
    {
        hilet non_uniform_scale = f32x4{dst_extent}.xyxy() / f32x4{src_extent}.xyxy();
        hilet uniform_scale = std::min(non_uniform_scale.x(), non_uniform_scale.y());
        return scale2{uniform_scale};
    }

    [[nodiscard]] constexpr friend vector2 operator*(scale2 const& lhs, vector2 const& rhs) noexcept
    {
        return vector2{lhs._v *f32x4{rhs}};
    }

    [[nodiscard]] constexpr friend extent2 operator*(scale2 const& lhs, extent2 const& rhs) noexcept
    {
        return extent2{lhs._v *f32x4{rhs}};
    }

    [[nodiscard]] constexpr friend point2 operator*(scale2 const& lhs, point2 const& rhs) noexcept
    {
        return point2{lhs._v *f32x4{rhs}};
    }

    /** Scale a rectangle around it's center.
     */
    [[nodiscard]] constexpr friend aarectangle operator*(scale2 const& lhs, aarectangle const& rhs) noexcept
    {
        return aarectangle{lhs * get<0>(rhs), lhs * get<3>(rhs)};
    }

    /** scale the quad.
     *
     * Each edge of the quad scaled.
     *
     * @param lhs A quad.
     * @param rhs The width and height to scale each edge with.
     * @return The new quad extended by the size.
     */
    [[nodiscard]] friend constexpr quad scale_from_center(quad const& lhs, scale2 const& rhs) noexcept
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

    [[nodiscard]] constexpr friend scale2 operator*(scale2 const& lhs, geo::identity const&) noexcept
    {
        return lhs;
    }

    [[nodiscard]] constexpr friend scale2 operator*(scale2 const& lhs, scale2 const& rhs) noexcept
    {
        return scale2{lhs._v * rhs._v};
    }

    [[nodiscard]] constexpr friend bool operator==(scale2 const& lhs, scale2 const& rhs) noexcept
    {
        return equal(lhs._v, rhs._v);
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.z() == 1.0f and _v.w() == 1.0f;
    }

private:
    array_type _v;
};

[[nodiscard]] constexpr matrix2
matrix2::uniform(aarectangle src_rectangle, aarectangle dst_rectangle, alignment alignment) noexcept
{
    hilet scale = scale2::uniform(src_rectangle.size(), dst_rectangle.size());
    hilet scaled_rectangle = scale * src_rectangle;
    hilet translation = translate2::align(scaled_rectangle, dst_rectangle, alignment);
    return translation * scale;
}

[[nodiscard]] constexpr scale2 operator/(extent2 const& lhs, extent2 const& rhs) noexcept
{
    hi_axiom(rhs.width() > 0.0f and rhs.height() > 0.0f);
    return scale2{f32x4{lhs}.xy11() / f32x4{rhs}.xy11()};
}

}} // namespace hi::v1
