// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include "TTauri/Foundation/sRGB.hpp"
#include <immintrin.h>
#include <emmintrin.h>
#include <algorithm>

namespace tt {

class A8B8G8R8SrgbPack32 {
    uint32_t v;

public:
    tt_force_inline A8B8G8R8SrgbPack32() = default;
    tt_force_inline A8B8G8R8SrgbPack32(A8B8G8R8SrgbPack32 const &rhs) noexcept = default;
    tt_force_inline A8B8G8R8SrgbPack32(A8B8G8R8SrgbPack32 &&rhs) noexcept = default;
    tt_force_inline A8B8G8R8SrgbPack32 &operator=(A8B8G8R8SrgbPack32 const &rhs) noexcept = default;
    tt_force_inline A8B8G8R8SrgbPack32 &operator=(A8B8G8R8SrgbPack32 &&rhs) noexcept = default;

    //tt_force_inline A8B8G8R8SrgbPack32(vec const &rhs) noexcept {
    //}

    //tt_force_inline A8B8G8R8SrgbPack32 &operator=(vec const &rhs) noexcept {
    //    return *this;
    //}

    //operator vec () const noexcept {
    //}

    tt_force_inline A8B8G8R8SrgbPack32(uint32_t const &rhs) noexcept : v(rhs) {}
    tt_force_inline A8B8G8R8SrgbPack32 &operator=(uint32_t const &rhs) noexcept { v = rhs; return *this; }
    tt_force_inline operator uint32_t () noexcept { return v; }

    tt_force_inline A8B8G8R8SrgbPack32(R16G16B16A16SFloat const &rhs) noexcept {
        ttlet &rhs_v = rhs.get();

        ttlet r = sRGB_linear16_to_gamma8(rhs_v[0].get());
        ttlet g = sRGB_linear16_to_gamma8(rhs_v[1].get());
        ttlet b = sRGB_linear16_to_gamma8(rhs_v[2].get());
        ttlet a = static_cast<uint8_t>(std::clamp(rhs_v[3] * 255.0f, 0.0f, 255.0f));
        v = (static_cast<uint32_t>(a) << 24) |
            (static_cast<uint32_t>(b) << 16) |
            (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(r);
    }

    tt_force_inline A8B8G8R8SrgbPack32 &operator=(R16G16B16A16SFloat const &rhs) noexcept {
        ttlet &rhs_v = rhs.get();

        ttlet r = sRGB_linear16_to_gamma8(rhs_v[0]);
        ttlet g = sRGB_linear16_to_gamma8(rhs_v[1]);
        ttlet b = sRGB_linear16_to_gamma8(rhs_v[2]);
        ttlet a = static_cast<uint8_t>(std::clamp(rhs_v[3] * 255.0f, 0.0f, 255.0f));
        v = (static_cast<uint32_t>(a) << 24) |
            (static_cast<uint32_t>(b) << 16) |
            (static_cast<uint32_t>(g) << 8) |
            static_cast<uint32_t>(r);
        return *this;
    }

    [[nodiscard]] tt_force_inline friend bool operator==(A8B8G8R8SrgbPack32 const &lhs, A8B8G8R8SrgbPack32 const &rhs) noexcept {
        return lhs.v == rhs.v;
    }
    [[nodiscard]] tt_force_inline friend bool operator!=(A8B8G8R8SrgbPack32 const &lhs, A8B8G8R8SrgbPack32 const &rhs) noexcept {
        return !(lhs == rhs);
    }

    [[nodiscard]] tt_force_inline friend A8B8G8R8SrgbPack32 makeTransparent(A8B8G8R8SrgbPack32 const &rhs) noexcept {
        return {rhs.v & 0x00ffffff};
    }
};



inline void fill(PixelMap<A8B8G8R8SrgbPack32>& dst, PixelMap<R16G16B16A16SFloat> const& src) noexcept
{
    tt_assert(dst.width >= src.width);
    tt_assert(dst.height >= src.height);

    for (auto rowNr = 0; rowNr < src.height; rowNr++) {
        ttlet srcRow = src.at(rowNr);
        auto dstRow = dst.at(rowNr);
        for (auto columnNr = 0; columnNr < src.width; columnNr++) {
            dstRow[columnNr] = srcRow[columnNr];
        }
    }
}


}
