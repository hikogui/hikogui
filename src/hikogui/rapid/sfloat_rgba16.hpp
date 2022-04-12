// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../color/color.hpp"
#include "../float16.hpp"
#include "../pixel_map.hpp"
#include "../geometry/corner_radii.hpp"
#include "../rapid/numeric_array.hpp"
#include "../hash.hpp"
#include <algorithm>
#include <bit>
#include <array>

namespace tt::inline v1 {

class sfloat_rgba16 {
    // Red, Green, Blue, Alpha in binary16 (native endian).
    std::array<float16, 4> v;

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

inline void fill(pixel_map<sfloat_rgba16> &image, f32x4 color) noexcept
{
    for (std::size_t y = 0; y != image.height(); ++y) {
        auto row = image[y];
        for (std::size_t x = 0; x != image.width(); ++x) {
            row[x] = color;
        }
    }
}

inline void composit(pixel_map<sfloat_rgba16> &under, pixel_map<sfloat_rgba16> const &over) noexcept
{
    tt_assert(over.height() >= under.height());
    tt_assert(over.width() >= under.width());

    for (std::size_t rowNr = 0; rowNr != under.height(); ++rowNr) {
        ttlet overRow = over.at(rowNr);
        auto underRow = under.at(rowNr);
        for (std::size_t columnNr = 0; columnNr != under.width(); ++columnNr) {
            ttlet &overPixel = overRow[columnNr];
            auto &underPixel = underRow[columnNr];

            underPixel = composit(static_cast<f16x4>(underPixel), static_cast<f16x4>(overPixel));
        }
    }
}

inline void composit(pixel_map<sfloat_rgba16> &under, color over, pixel_map<uint8_t> const &mask) noexcept
{
    tt_assert(mask.height() >= under.height());
    tt_assert(mask.width() >= under.width());

    auto maskPixel = color{1.0f, 1.0f, 1.0f, 1.0f};

    for (std::size_t rowNr = 0; rowNr != under.height(); ++rowNr) {
        ttlet maskRow = mask.at(rowNr);
        auto underRow = under.at(rowNr);
        for (std::size_t columnNr = 0; columnNr != under.width(); ++columnNr) {
            ttlet maskValue = maskRow[columnNr] / 255.0f;
            maskPixel.a() = maskValue;

            auto &pixel = underRow[columnNr];
            pixel = composit(static_cast<color>(pixel), over * maskPixel);
        }
    }
}

} // namespace tt::inline v1

template<>
struct std::hash<tt::sfloat_rgba16> {
    std::size_t operator()(tt::sfloat_rgba16 const &rhs) const noexcept
    {
        return rhs.hash();
    }
};
