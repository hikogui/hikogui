// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/PixelMap.inl"
#include "TTauri/Foundation/endian.hpp"
#include <algorithm>

namespace TTauri {





void mergeMaximum(PixelMap<uint8_t> &dst, PixelMap<uint8_t> const &src) noexcept
{
    ttauri_assert(src.width >= dst.width);
    ttauri_assert(src.height >= dst.height);

    for (auto rowNr = 0; rowNr < dst.height; rowNr++) {
        auto dstRow = dst[rowNr];
        let srcRow = src[rowNr];
        for (auto columnNr = 0; columnNr < dstRow.width; columnNr++) {
            auto &dstPixel = dstRow[columnNr];
            let srcPixel = srcRow[columnNr];
            dstPixel = std::max(dstPixel, srcPixel);
        }
    }
}

}
