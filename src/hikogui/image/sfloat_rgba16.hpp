// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file image/sfloat_rgba16.hpp Defines the sfloat_rgba16.
 * @ingroup image
 */

#pragma once

#include "pixmap_span.hpp"
#include "../color/color.hpp"
#include "../geometry/geometry.hpp"
#include "../SIMD/SIMD.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <algorithm>
#include <bit>
#include <cstdint>
#include <array>

hi_export_module(hikogui.image.sfloat_rgba16);

hi_export namespace hi::inline v1 {

/** 4 x half pixel format.
 *
 * @ingroup image
 */
class sfloat_rgba16 {
    // Red, Green, Blue, Alpha in binary16 (native endian).
    std::array<half, 4> v;

public:
    constexpr sfloat_rgba16() noexcept : v() {}

    constexpr sfloat_rgba16(sfloat_rgba16 const &rhs) noexcept = default;
    constexpr sfloat_rgba16(sfloat_rgba16 &&rhs) noexcept = default;
    constexpr sfloat_rgba16 &operator=(sfloat_rgba16 const &rhs) noexcept = default;
    constexpr sfloat_rgba16 &operator=(sfloat_rgba16 &&rhs) noexcept = default;

    constexpr sfloat_rgba16(f16x4 const &rhs) noexcept : v(std::bit_cast<decltype(v)>(rhs)) {}

    constexpr sfloat_rgba16 &operator=(f16x4 const &rhs) noexcept
    {
        v = std::bit_cast<decltype(v)>(rhs);
        return *this;
    }

    constexpr explicit operator f16x4() const noexcept
    {
        return std::bit_cast<f16x4>(v);
    }

    constexpr sfloat_rgba16(f32x4 const &rhs) noexcept : sfloat_rgba16(static_cast<f16x4>(rhs)) {}

    constexpr sfloat_rgba16 &operator=(f32x4 const &rhs) noexcept
    {
        return *this = static_cast<f16x4>(rhs);
    }

    constexpr sfloat_rgba16(color const &rhs) noexcept : sfloat_rgba16(static_cast<f16x4>(rhs)) {}

    constexpr sfloat_rgba16 &operator=(color const &rhs) noexcept
    {
        return *this = static_cast<f16x4>(rhs);
    }

    constexpr explicit operator color() const noexcept
    {
        return color{static_cast<f16x4>(*this)};
    }

    [[nodiscard]] constexpr sfloat_rgba16(corner_radii const &rhs) noexcept : sfloat_rgba16(static_cast<f32x4>(rhs)) {}

    [[nodiscard]] std::size_t hash() const noexcept
    {
        return hash_mix(v[0], v[1], v[2], v[3]);
    }

    [[nodiscard]] constexpr friend bool operator==(sfloat_rgba16 const &lhs, sfloat_rgba16 const &rhs) noexcept = default;

    [[nodiscard]] friend sfloat_rgba16 make_transparent(sfloat_rgba16 const &rhs) noexcept
    {
        sfloat_rgba16 r;
        r.v = rhs.v;
        std::get<3>(r.v) = 0x0000; // 0.0f
        return r;
    }
};

constexpr void fill(pixmap_span<sfloat_rgba16> image, f32x4 color) noexcept
{
    for (std::size_t y = 0; y != image.height(); ++y) {
        hilet row = image[y];
        for (std::size_t x = 0; x != image.width(); ++x) {
            row[x] = color;
        }
    }
}

hi_inline void composit(pixmap_span<sfloat_rgba16> under, pixmap_span<sfloat_rgba16 const> over) noexcept
{
    hi_assert(over.height() >= under.height());
    hi_assert(over.width() >= under.width());

    for (auto y = 0_uz; y != under.height(); ++y) {
        hilet over_line = over[y];
        hilet under_line = under[y];
        for (auto x = 0_uz; x != under.width(); ++x) {
            hilet &overPixel = over_line[x];
            auto &underPixel = under_line[x];

            underPixel = composit(static_cast<f16x4>(underPixel), static_cast<f16x4>(overPixel));
        }
    }
}

hi_inline void composit(pixmap_span<sfloat_rgba16> under, color over, pixmap_span<uint8_t const> mask) noexcept
{
    hi_assert(mask.height() >= under.height());
    hi_assert(mask.width() >= under.width());

    auto mask_pixel = color{1.0f, 1.0f, 1.0f, 1.0f};

    for (auto y = 0_uz; y != under.height(); ++y) {
        hilet mask_line = mask[y];
        hilet under_line = under[y];
        for (auto x = 0_uz; x != under.width(); ++x) {
            hilet mask_value = mask_line[x] / 255.0f;
            mask_pixel.a() = mask_value;

            auto &pixel = under_line[x];
            pixel = composit(static_cast<color>(pixel), over * mask_pixel);
        }
    }
}

} // namespace hi::inline v1

template<>
struct std::hash<hi::sfloat_rgba16> {
    std::size_t operator()(hi::sfloat_rgba16 const &rhs) const noexcept
    {
        return rhs.hash();
    }
};
