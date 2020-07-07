// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/PixelMap.inl"
#include "ttauri/foundation/endian.hpp"
#include <algorithm>

namespace tt {





void mergeMaximum(PixelMap<uint8_t> &dst, PixelMap<uint8_t> const &src) noexcept
{
    tt_assert(src.width >= dst.width);
    tt_assert(src.height >= dst.height);

    for (auto rowNr = 0; rowNr < dst.height; rowNr++) {
        auto dstRow = dst[rowNr];
        ttlet srcRow = src[rowNr];
        for (auto columnNr = 0; columnNr < dstRow.width; columnNr++) {
            auto &dstPixel = dstRow[columnNr];
            ttlet srcPixel = srcRow[columnNr];
            dstPixel = std::max(dstPixel, srcPixel);
        }
    }
}

}
