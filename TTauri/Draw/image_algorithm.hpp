// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PixelMap.hpp"
#include "TTauri/Color.hpp"
#include <algorithm>

namespace TTauri::Draw {

/*! Make the pixel around the border transparent.
 * But copy the color information from the neighbour pixel so that linear
 * interpolation near the border will work propertly.
 */
inline void add1PixelTransparentBorder(PixelMap<uint32_t>& pixelMap)
{
    uint8_t const u8invisibleMask[4] = { 0xff, 0xff, 0xff, 0 };
    uint32_t u32invisibleMask;
    std::memcpy(&u32invisibleMask, u8invisibleMask, sizeof(u32invisibleMask));

    auto topBorder = pixelMap.at(0);
    let topRow = pixelMap.at(1);
    let bottomRow = pixelMap.at(pixelMap.height - 2);
    auto bottomBorder = pixelMap.at(pixelMap.height - 1);
    for (size_t x = 1; x < pixelMap.width - 1; x++) {
        topBorder[x] = topRow[x] & u32invisibleMask;
        bottomBorder[x] = bottomRow[x] & u32invisibleMask;
    }

    let rightBorderY = pixelMap.width - 1;
    let rightY = pixelMap.width - 2;
    for (size_t y = 1; y < pixelMap.height - 1; y++) {
        auto row = pixelMap[y];
        row[0] = row[1] & u32invisibleMask;
        row[rightBorderY] = row[rightY] & u32invisibleMask;
    }

    pixelMap[0][0] = pixelMap[1][1] & u32invisibleMask;
    pixelMap[0][pixelMap.width - 1] = pixelMap[1][pixelMap.width - 2] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][0] = pixelMap[pixelMap.height - 2][1] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][pixelMap.width - 1] = pixelMap[pixelMap.height - 2][pixelMap.width - 2] & u32invisibleMask;
}

/*! Copy a image with linear 16bit-per-color-component to a
 * gamma corrected 8bit-per-color-component image.
 */
inline void copyLinearToGamma(PixelMap<uint32_t>& dst, PixelMap<wsRGBApm> const& src)
{
    assert(dst.width >= src.width);
    assert(dst.height >= src.height);

    for (size_t rowNr = 0; rowNr < src.height; rowNr++) {
        let srcRow = src.at(rowNr);
        auto dstRow = dst.at(rowNr);
        for (size_t columnNr = 0; columnNr < src.width; columnNr++) {
            dstRow[columnNr] = boost::endian::native_to_big(srcRow[columnNr].to_sRGBApm_u32());
        }
    }
}

template<int KERNEL_SIZE, typename KERNEL>
inline void horizontalFilterRow(PixelRow<uint8_t> row, KERNEL kernel) {
    constexpr auto LOOK_AHEAD_SIZE = KERNEL_SIZE / 2;

    uint64_t values = 0;
    int64_t x;

    // Start beyond the left pixel. Then lookahead upto
    // the point we can start the kernel.
    let leftEdgeValue = row[0];
    for (x = -KERNEL_SIZE; x < 0; x++) {
        values <<= 8;

        if ((LOOK_AHEAD_SIZE + x) < 0) {
            values |= leftEdgeValue;
        }
        else {
            values |= row[LOOK_AHEAD_SIZE + x];
        }
    }

    // Execute the kernel on all the pixels upto the right edge.
    // The values are still looked up ahead.
    int64_t const lastX = row.width - LOOK_AHEAD_SIZE;
    for (; x < lastX; x++) {
        values <<= 8;
        values |= row[LOOK_AHEAD_SIZE + x];

        row[x] = kernel(values);
    }

    // Finish up to the right edge.
    let rightEdgeValue = row[row.width - 1];
    for (; x < static_cast<int64_t>(row.width); x++) {
        values <<= 8;
        values |= rightEdgeValue;

        row[x] = kernel(values);
    }
}


template<int KERNEL_SIZE, typename T, typename KERNEL>
inline void horizontalFilter(PixelMap<T>& pixels, KERNEL kernel) {
    for (size_t rowNr = 0; rowNr < pixels.height; rowNr++) {
        auto row = pixels.at(rowNr);
        horizontalFilterRow<KERNEL_SIZE>(row, kernel);
    }
}

/*! Composit the color `over` onto the image `under` based on the pixel mask.
 * Mask should be passed to subpixelFilter() before use.
 */
inline void composit(PixelMap<wsRGBApm>& under, wsRGBApm over, PixelMap<uint8_t> const& mask)
{
    assert(mask.height >= under.height);
    assert(mask.width >= under.width);

    for (size_t rowNr = 0; rowNr < under.height; rowNr++) {
        let maskRow = mask.at(rowNr);
        auto underRow = under.at(rowNr);
        for (size_t columnNr = 0; columnNr < static_cast<size_t>(underRow.width); columnNr++) {
            let maskPixel = maskRow[columnNr];
            auto& pixel = underRow[columnNr];
            pixel.composit(over, maskPixel);
        }
    }
}

/*! Composit the color `over` onto the image `under` based on the subpixel mask.
 * Mask should be passed to subpixelFilter() before use.
 */
inline void subpixelComposit(PixelMap<wsRGBApm>& under, wsRGBApm over, PixelMap<uint8_t> const& mask)
{
    assert(mask.height >= under.height);
    assert((mask.width * 3) >= under.width);

    for (size_t rowNr = 0; rowNr < under.height; rowNr++) {
        let maskRow = mask.at(rowNr);
        auto underRow = under.at(rowNr);
        for (size_t maskColumnNr = 0, columnNr = 0; columnNr < static_cast<size_t>(underRow.width); columnNr++, maskColumnNr += 3) {
            let maskRGBValue = glm::u8vec3{
                maskRow[maskColumnNr],
                maskRow[maskColumnNr + 1],
                maskRow[maskColumnNr + 2]
            };

            auto& pixel = underRow[columnNr];
            pixel.subpixelComposit(over, maskRGBValue);
        }
    }
}

inline void subpixelFilter(PixelMap<uint8_t> &image) {
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

/*! Swap R and B values of each RGB pixel.
 */
inline void subpixelFlip(PixelMap<uint8_t> &image) {
    assert(image.width % 3 == 0);

    for (size_t rowNr = 0; rowNr < image.height; rowNr++) {
        auto row = image.at(rowNr);
        for (size_t columnNr = 0; columnNr < static_cast<size_t>(row.width); columnNr += 3) {
            std::swap(row[columnNr], row[columnNr + 2]);
        }
    }
}

}
