// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "vector2.hpp"
#include "extent2.hpp"
#include "../macros.hpp"
#include <algorithm>
#include <exception>
#include <compare>

hi_export_module(hikogui.geometry : scale2);

hi_export namespace hi { inline namespace v1 {

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

    [[nodiscard]] constexpr scale2() noexcept : _v(1.0f, 1.0f, 1.0f, 1.0f) {}

    [[nodiscard]] constexpr scale2(float value) noexcept : _v(value, value, value, 1.0f) {}

    [[nodiscard]] constexpr scale2(float x, float y) noexcept : _v(x, y, 1.0f, 1.0f) {}

    /** Get a uniform-scale-transform to scale an extent to another extent.
     * @param src_extent The extent to transform
     * @param dst_extent The extent to scale to.
     * @return a scale to transform the src_extent to the dst_extent.
     */
    [[nodiscard]] constexpr static scale2 uniform(extent2 src_extent, extent2 dst_extent) noexcept
    {
        hilet non_uniform_scale = f32x4{dst_extent}.xyxy() / f32x4{src_extent}.xyxy();
        hilet uniform_scale = std::min(non_uniform_scale.x(), non_uniform_scale.y());
        return scale2{uniform_scale};
    }

    [[nodiscard]] constexpr friend bool operator==(scale2 const& lhs, scale2 const& rhs) noexcept
    {
        return equal(lhs._v, rhs._v);
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.z() == 1.0f and _v.w() == 1.0f;
    }

    [[nodiscard]] constexpr float& x() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float& y() noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float x() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float y() const noexcept
    {
        return _v.y();
    }

private:
    array_type _v;
};

[[nodiscard]] constexpr scale2 operator/(extent2 const& lhs, extent2 const& rhs) noexcept
{
    hi_axiom(rhs.width() > 0.0f and rhs.height() > 0.0f);
    return scale2{f32x4{lhs}.xy11() / f32x4{rhs}.xy11()};
}

}} // namespace hi::v1
