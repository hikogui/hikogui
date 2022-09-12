// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "sfloat_rgba16.hpp"
#include "../color/sRGB.hpp"
#include <algorithm>

namespace hi::inline v1 {

class srgb_abgr8_pack {
    uint32_t v;

public:
    constexpr srgb_abgr8_pack() = default;
    constexpr srgb_abgr8_pack(srgb_abgr8_pack const &rhs) noexcept = default;
    constexpr srgb_abgr8_pack(srgb_abgr8_pack &&rhs) noexcept = default;
    constexpr srgb_abgr8_pack &operator=(srgb_abgr8_pack const &rhs) noexcept = default;
    constexpr srgb_abgr8_pack &operator=(srgb_abgr8_pack &&rhs) noexcept = default;

    constexpr srgb_abgr8_pack(uint32_t const &rhs) noexcept : v(rhs) {}
    constexpr srgb_abgr8_pack &operator=(uint32_t const &rhs) noexcept
    {
        v = rhs;
        return *this;
    }
    constexpr operator uint32_t() noexcept
    {
        return v;
    }

    constexpr srgb_abgr8_pack(sfloat_rgba16 const &rhs) noexcept
    {
        hilet &rhs_v = rhs.get();

        hilet r = sRGB_linear16_to_gamma8(rhs_v[0].get());
        hilet g = sRGB_linear16_to_gamma8(rhs_v[1].get());
        hilet b = sRGB_linear16_to_gamma8(rhs_v[2].get());
        hilet a = static_cast<uint8_t>(std::clamp(rhs_v[3] * 255.0f, 0.0f, 255.0f));
        v = (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(r);
    }

    constexpr srgb_abgr8_pack &operator=(sfloat_rgba16 const &rhs) noexcept
    {
        hilet &rhs_v = rhs.get();

        hilet r = sRGB_linear16_to_gamma8(rhs_v[0]);
        hilet g = sRGB_linear16_to_gamma8(rhs_v[1]);
        hilet b = sRGB_linear16_to_gamma8(rhs_v[2]);
        hilet a = static_cast<uint8_t>(std::clamp(rhs_v[3] * 255.0f, 0.0f, 255.0f));
        v = (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(r);
        return *this;
    }

    [[nodiscard]] constexpr friend bool operator==(srgb_abgr8_pack const &lhs, srgb_abgr8_pack const &rhs) noexcept = default;

    [[nodiscard]] constexpr friend srgb_abgr8_pack makeTransparent(srgb_abgr8_pack const &rhs) noexcept
    {
        return {rhs.v & 0x00ffffff};
    }
};

inline void fill(pixel_map<srgb_abgr8_pack> &dst, pixel_map<sfloat_rgba16> const &src) noexcept
{
    hi_assert(dst.width >= src.width);
    hi_assert(dst.height >= src.height);

    for (auto rowNr = 0; rowNr < src.height; rowNr++) {
        hilet srcRow = src.at(rowNr);
        auto dstRow = dst.at(rowNr);
        for (auto columnNr = 0; columnNr < src.width; columnNr++) {
            dstRow[columnNr] = srcRow[columnNr];
        }
    }
}

} // namespace hi::inline v1
