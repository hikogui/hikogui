// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "vec.hpp"
#include "float16.hpp"
#include "PixelMap.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class R16G16B16A16SFloat {
    // Red, Green, Blue, Alpha in binary16 (native endian).
    std::array<float16,4> v;

public:
    tt_force_inline R16G16B16A16SFloat() noexcept {
        std::memset(v.data(), 0, sizeof(v));
    }

    tt_force_inline R16G16B16A16SFloat(R16G16B16A16SFloat const &rhs) noexcept = default;
    tt_force_inline R16G16B16A16SFloat(R16G16B16A16SFloat &&rhs) noexcept = default;
    tt_force_inline R16G16B16A16SFloat &operator=(R16G16B16A16SFloat const &rhs) noexcept = default;
    tt_force_inline R16G16B16A16SFloat &operator=(R16G16B16A16SFloat &&rhs) noexcept = default;

    tt_force_inline R16G16B16A16SFloat(vec const &rhs) noexcept {
        ttlet rhs_fp16 = _mm_cvtps_ph(rhs, _MM_FROUND_CUR_DIRECTION);
        _mm_storeu_si64(v.data(), rhs_fp16);
    }

    tt_force_inline R16G16B16A16SFloat &operator=(vec const &rhs) noexcept {
        ttlet rhs_fp16 = _mm_cvtps_ph(rhs, _MM_FROUND_CUR_DIRECTION);
        _mm_storeu_si64(v.data(), rhs_fp16);
        return *this;
    }

    tt_force_inline operator vec () const noexcept {
        ttlet rhs_fp16 = _mm_loadu_si64(v.data());
        return _mm_cvtph_ps(rhs_fp16);
    }

    std::array<float16,4> const &get() const noexcept {
        return v;
    }

    std::array<float16,4> &get() noexcept {
        return v;
    }

    [[nodiscard]] tt_force_inline friend bool operator==(R16G16B16A16SFloat const &lhs, R16G16B16A16SFloat const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] tt_force_inline friend bool operator!=(R16G16B16A16SFloat const &lhs, R16G16B16A16SFloat const &rhs) noexcept {
        return !(lhs == rhs);
    }

    [[nodiscard]] tt_force_inline friend R16G16B16A16SFloat makeTransparent(R16G16B16A16SFloat const &rhs) noexcept {
        R16G16B16A16SFloat r;
        r.v = rhs.v;
        std::get<3>(r.v) = 0x0000; // 0.0f
        return r;
    }
};

inline void fill(PixelMap<R16G16B16A16SFloat> &image, vec color) noexcept {
    for (ssize_t y = 0; y != image.height; ++y) {
        auto row = image[y];
        for (ssize_t x = 0; x != image.width; ++x) {
            row[x] = color;
        }
    }
}

inline void desaturate(PixelMap<R16G16B16A16SFloat> &image, float brightness) noexcept {
    for (ssize_t y = 0; y != image.height; ++y) {
        auto row = image[y];
        for (ssize_t x = 0; x != image.width; ++x) {
            row[x] = desaturate(static_cast<vec>(row[x]), brightness);
        }
    }
}

inline void composit(PixelMap<R16G16B16A16SFloat> &under, PixelMap<R16G16B16A16SFloat> const &over) noexcept
{
    tt_assert(over.height >= under.height);
    tt_assert(over.width >= under.width);

    for (ssize_t rowNr = 0; rowNr != under.height; ++rowNr) {
        ttlet overRow = over.at(rowNr);
        auto underRow = under.at(rowNr);
        for (ssize_t columnNr = 0; columnNr != under.width; ++columnNr) {
            ttlet &overPixel = overRow[columnNr];
            auto &underPixel = underRow[columnNr];

            underPixel = composit(static_cast<vec>(underPixel), static_cast<vec>(overPixel));
        }
    }
}

inline void composit(PixelMap<R16G16B16A16SFloat>& under, vec over, PixelMap<uint8_t> const& mask) noexcept
{
    tt_assert(mask.height >= under.height);
    tt_assert(mask.width >= under.width);

    auto maskPixel = vec::color(1.0f, 1.0f, 1.0f, 1.0f);

    for (ssize_t rowNr = 0; rowNr != under.height; ++rowNr) {
        ttlet maskRow = mask.at(rowNr);
        auto underRow = under.at(rowNr);
        for (ssize_t columnNr = 0; columnNr != under.width; ++columnNr) {
            ttlet maskValue = maskRow[columnNr] / 255.0f;
            maskPixel.a(maskValue);

            auto& pixel = underRow[columnNr];
            pixel = composit(pixel, over * maskPixel);
        }
    }
}

}
