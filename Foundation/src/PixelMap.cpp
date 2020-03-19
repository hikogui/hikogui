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

void subpixelFilter(PixelMap<uint8_t> &image) noexcept
{
    horizontalFilter<5>(image, [](auto values) {
        return static_cast<uint8_t>((
            (values & 0xff) +
            ((values >> 8) & 0xff) * 2 +
            ((values >> 16) & 0xff) * 3 +
            ((values >> 24) & 0xff) * 2 +
            ((values >> 32) & 0xff)
            ) / 9);
        }
    );
}

void subpixelFlip(PixelMap<uint8_t> &image) noexcept
{
    ttauri_assert(image.width % 3 == 0);

    for (auto rowNr = 0; rowNr < image.height; rowNr++) {
        auto row = image.at(rowNr);
        for (auto columnNr = 0; columnNr < static_cast<size_t>(row.width); columnNr += 3) {
            std::swap(row[columnNr], row[columnNr + 2]);
        }
    }
}

}
