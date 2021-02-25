// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../numeric_array.hpp"
#include "color.hpp"
#include "../float16.hpp"
#include "../pixel_map.hpp"
#include "../geometry/corner_shapes.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class sfloat_rgba16 {
    // Red, Green, Blue, Alpha in binary16 (native endian).
    std::array<float16, 4> v;

public:
    sfloat_rgba16() noexcept
    {
        std::memset(v.data(), 0, sizeof(v));
    }

    sfloat_rgba16(sfloat_rgba16 const &rhs) noexcept = default;
    sfloat_rgba16(sfloat_rgba16 &&rhs) noexcept = default;
    sfloat_rgba16 &operator=(sfloat_rgba16 const &rhs) noexcept = default;
    sfloat_rgba16 &operator=(sfloat_rgba16 &&rhs) noexcept = default;

    sfloat_rgba16(f32x4 const &rhs) noexcept
    {
        ttlet rhs_fp16 = _mm_cvtps_ph(static_cast<__m128>(rhs), _MM_FROUND_CUR_DIRECTION);
        _mm_storeu_si64(v.data(), rhs_fp16);
    }

    sfloat_rgba16 &operator=(f32x4 const &rhs) noexcept
    {
        ttlet rhs_fp16 = _mm_cvtps_ph(static_cast<__m128>(rhs), _MM_FROUND_CUR_DIRECTION);
        _mm_storeu_si64(v.data(), rhs_fp16);
        return *this;
    }

    explicit operator f32x4() const noexcept
    {
        ttlet rhs_fp16 = _mm_loadu_si64(v.data());
        return f32x4{_mm_cvtph_ps(rhs_fp16)};
    }

    sfloat_rgba16(color const &rhs) noexcept : sfloat_rgba16(static_cast<f32x4>(rhs)) {}

    sfloat_rgba16 &operator=(color const &rhs) noexcept
    {
        return *this = static_cast<f32x4>(rhs);
    }

    explicit operator color() const noexcept
    {
        return color{static_cast<f32x4>(*this)};
    }

    [[nodiscard]] sfloat_rgba16(corner_shapes const &rhs) noexcept : sfloat_rgba16(static_cast<f32x4>(rhs)) {}

    // std::array<float16,4> const &get() const noexcept {
    //    return v;
    //}
    //
    // std::array<float16,4> &get() noexcept {
    //    return v;
    //}

    [[nodiscard]] friend bool operator==(sfloat_rgba16 const &lhs, sfloat_rgba16 const &rhs) noexcept
    {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] friend bool operator!=(sfloat_rgba16 const &lhs, sfloat_rgba16 const &rhs) noexcept
    {
        return !(lhs == rhs);
    }

    [[nodiscard]] friend sfloat_rgba16 makeTransparent(sfloat_rgba16 const &rhs) noexcept
    {
        sfloat_rgba16 r;
        r.v = rhs.v;
        std::get<3>(r.v) = 0x0000; // 0.0f
        return r;
    }
};

inline void fill(pixel_map<sfloat_rgba16> &image, f32x4 color) noexcept
{
    for (ssize_t y = 0; y != image.height(); ++y) {
        auto row = image[y];
        for (ssize_t x = 0; x != image.width(); ++x) {
            row[x] = color;
        }
    }
}

inline void desaturate(pixel_map<sfloat_rgba16> &image, float brightness) noexcept
{
    for (ssize_t y = 0; y != image.height(); ++y) {
        auto row = image[y];
        for (ssize_t x = 0; x != image.width(); ++x) {
            row[x] = desaturate(static_cast<f32x4>(row[x]), brightness);
        }
    }
}

inline void composit(pixel_map<sfloat_rgba16> &under, pixel_map<sfloat_rgba16> const &over) noexcept
{
    tt_assert(over.height() >= under.height());
    tt_assert(over.width() >= under.width());

    for (ssize_t rowNr = 0; rowNr != under.height(); ++rowNr) {
        ttlet overRow = over.at(rowNr);
        auto underRow = under.at(rowNr);
        for (ssize_t columnNr = 0; columnNr != under.width(); ++columnNr) {
            ttlet &overPixel = overRow[columnNr];
            auto &underPixel = underRow[columnNr];

            underPixel = composit(static_cast<f32x4>(underPixel), static_cast<f32x4>(overPixel));
        }
    }
}

inline void composit(pixel_map<sfloat_rgba16> &under, color over, pixel_map<uint8_t> const &mask) noexcept
{
    tt_assert(mask.height() >= under.height());
    tt_assert(mask.width() >= under.width());

    auto maskPixel = color{1.0f, 1.0f, 1.0f, 1.0f};

    for (ssize_t rowNr = 0; rowNr != under.height(); ++rowNr) {
        ttlet maskRow = mask.at(rowNr);
        auto underRow = under.at(rowNr);
        for (ssize_t columnNr = 0; columnNr != under.width(); ++columnNr) {
            ttlet maskValue = maskRow[columnNr] / 255.0f;
            maskPixel.a() = maskValue;

            auto &pixel = underRow[columnNr];
            pixel = composit(static_cast<color>(pixel), over * maskPixel);
        }
    }
}

} // namespace tt
