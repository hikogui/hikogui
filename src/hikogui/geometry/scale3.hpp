// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "scale2.hpp"
#include "matrix2.hpp"
#include "matrix3.hpp"
#include "gidentity.hpp"
#include "translate2.hpp"
#include "translate3.hpp"
#include "extent2.hpp"
#include "extent3.hpp"

namespace hi { inline namespace v1 {

class scale3 {
public:
    using array_type = simd<float, 4>;
    using value_type = array_type::value_type;

    constexpr scale3(scale3 const&) noexcept = default;
    constexpr scale3(scale3&&) noexcept = default;
    constexpr scale3& operator=(scale3 const&) noexcept = default;
    constexpr scale3& operator=(scale3&&) noexcept = default;

    [[nodiscard]] constexpr scale3(scale2 const& other) noexcept : _v(f32x4{other}) {}

    [[nodiscard]] constexpr explicit operator scale2() const noexcept
    {
        auto tmp = _v;
        tmp.z() = 1.0f;
        return scale2{tmp};
    }

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr explicit scale3(f32x4 const& v) noexcept : _v(v)
    {
        hi_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr explicit scale3(vector3 const& v) noexcept : _v(static_cast<f32x4>(v).xyz1()) {}

    [[nodiscard]] constexpr operator matrix3() const noexcept
    {
        return matrix3{_v.x000(), _v._0y00(), _v._00z0(), _v._000w()};
    }

    [[nodiscard]] constexpr scale3() noexcept : _v(1.0f, 1.0f, 1.0f, 1.0f) {}

    [[nodiscard]] constexpr scale3(gidentity const&) noexcept : _v(1.0f, 1.0f, 1.0f, 1.0f) {}

    [[nodiscard]] constexpr scale3(float value) noexcept : _v(value, value, value, 1.0f) {}

    [[nodiscard]] constexpr scale3(float x, float y, float z = 1.0f) noexcept : _v(x, y, z, 1.0f) {}

    /** Get a uniform-scale-transform to scale an extent to another extent.
     * @param src_extent The extent to transform
     * @param dst_extent The extent to scale to.
     * @return a scale to transform the src_extent to the dst_extent.
     */
    [[nodiscard]] static constexpr scale3 uniform(extent3 src_extent, extent3 dst_extent) noexcept
    {
        hilet non_uniform_scale = static_cast<f32x4>(dst_extent).xyzx() / static_cast<f32x4>(src_extent).xyzx();
        hilet uniform_scale = std::min({non_uniform_scale.x(), non_uniform_scale.y(), non_uniform_scale.z()});
        return scale3{uniform_scale};
    }

    [[nodiscard]] constexpr friend vector2 operator*(scale3 const& lhs, vector2 const& rhs) noexcept
    {
        return vector2{f32x4{lhs} * f32x4{rhs}};
    }

    [[nodiscard]] constexpr friend extent2 operator*(scale3 const& lhs, extent2 const& rhs) noexcept
    {
        return extent2{f32x4{lhs} * f32x4{rhs}};
    }

    [[nodiscard]] constexpr friend point2 operator*(scale3 const& lhs, point2 const& rhs) noexcept
    {
        return point2{f32x4{lhs} * f32x4{rhs}};
    }

    [[nodiscard]] constexpr friend scale3 operator*(scale3 const& lhs, gidentity const&) noexcept
    {
        return lhs;
    }

    [[nodiscard]] constexpr friend scale3 operator*(scale3 const& lhs, scale3 const& rhs) noexcept
    {
        return scale3{lhs._v * rhs._v};
    }

    [[nodiscard]] constexpr friend bool operator==(scale3 const& lhs, scale3 const& rhs) noexcept
    {
        return equal(lhs._v, rhs._v);
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() == 1.0f;
    }

private:
    array_type _v;
};

[[nodiscard]] constexpr scale3 operator/(extent3 const& lhs, extent3 const& rhs) noexcept
{
    return scale3{f32x4{lhs}.xyz1() / f32x4{rhs}.xyz1()};
}

[[nodiscard]] constexpr vector3 operator*(scale3 const& lhs, vector3 const& rhs) noexcept
{
    return vector3{f32x4{lhs} * f32x4{rhs}};
}

[[nodiscard]] constexpr extent3 operator*(scale3 const& lhs, extent3 const& rhs) noexcept
{
    return extent3{f32x4{lhs} * f32x4{rhs}};
}

[[nodiscard]] constexpr point3 operator*(scale3 const& lhs, point3 const& rhs) noexcept
{
    return point3{f32x4{lhs} * f32x4{rhs}};
}

[[nodiscard]] constexpr rectangle operator*(scale3 const& lhs, rectangle const& rhs) noexcept
{
    return rectangle{lhs * get<0>(rhs), lhs * get<1>(rhs), lhs * get<2>(rhs), lhs * get<3>(rhs)};
}

[[nodiscard]] constexpr quad operator*(scale3 const& lhs, quad const& rhs) noexcept
{
    return quad{lhs * rhs.p0, lhs * rhs.p1, lhs * rhs.p2, lhs * rhs.p3};
}

}} // namespace hi::v1
