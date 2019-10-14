// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Draw/PixelMap.inl"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/endian.hpp"
#include <algorithm>

namespace TTauri::Draw {

gsl_suppress(bounds.4)
void addTransparentBorder(PixelMap<uint32_t>& pixelMap) noexcept
{
    uint8_t const u8invisibleMask[4] = { 0xff, 0xff, 0xff, 0 };
    uint32_t u32invisibleMask;
    std::memcpy(&u32invisibleMask, u8invisibleMask, sizeof(u32invisibleMask));

    auto topBorder = pixelMap.at(0);
    let topRow = pixelMap.at(1);
    let bottomRow = pixelMap.at(pixelMap.height - 2);
    auto bottomBorder = pixelMap.at(pixelMap.height - 1);
    for (auto x = 1; x < pixelMap.width - 1; x++) {
        topBorder[x] = topRow[x] & u32invisibleMask;
        bottomBorder[x] = bottomRow[x] & u32invisibleMask;
    }

    let rightBorderY = pixelMap.width - 1;
    let rightY = pixelMap.width - 2;
    for (auto y = 1; y < pixelMap.height - 1; y++) {
        auto row = pixelMap[y];
        row[0] = row[1] & u32invisibleMask;
        row[rightBorderY] = row[rightY] & u32invisibleMask;
    }

    pixelMap[0][0] = pixelMap[1][1] & u32invisibleMask;
    pixelMap[0][pixelMap.width - 1] = pixelMap[1][pixelMap.width - 2] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][0] = pixelMap[pixelMap.height - 2][1] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][pixelMap.width - 1] = pixelMap[pixelMap.height - 2][pixelMap.width - 2] & u32invisibleMask;
}

void fill(PixelMap<uint32_t>& dst, PixelMap<wsRGBA> const& src) noexcept
{
    required_assert(dst.width >= src.width);
    required_assert(dst.height >= src.height);

    for (auto rowNr = 0; rowNr < src.height; rowNr++) {
        let srcRow = src.at(rowNr);
        auto dstRow = dst.at(rowNr);
        for (auto columnNr = 0; columnNr < src.width; columnNr++) {
            dstRow[columnNr] = native_to_big(srcRow[columnNr].to_sRGBA_u32());
        }
    }
}

void mergeMaximum(PixelMap<uint8_t> &dst, PixelMap<uint8_t> const &src) noexcept
{
    required_assert(src.width >= dst.width);
    required_assert(src.height >= dst.height);

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

void composit(PixelMap<wsRGBA> &under, PixelMap<wsRGBA> const &over) noexcept
{
    required_assert(over.height >= under.height);
    required_assert(over.width >= under.width);

    for (auto rowNr = 0; rowNr < under.height; rowNr++) {
        let overRow = over.at(rowNr);
        auto underRow = under.at(rowNr);
        for (auto columnNr = 0; columnNr < static_cast<size_t>(underRow.width); columnNr++) {
            let overPixel = overRow[columnNr];
            auto& underPixel = underRow[columnNr];
            underPixel.composit(overPixel);
        }
    }
}

void composit(PixelMap<wsRGBA>& under, wsRGBA over, PixelMap<uint8_t> const& mask) noexcept
{
    required_assert(mask.height >= under.height);
    required_assert(mask.width >= under.width);

    for (auto rowNr = 0; rowNr < under.height; rowNr++) {
        let maskRow = mask.at(rowNr);
        auto underRow = under.at(rowNr);
        for (auto columnNr = 0; columnNr < static_cast<size_t>(underRow.width); columnNr++) {
            let maskPixel = maskRow[columnNr];
            auto& pixel = underRow[columnNr];
            pixel.composit(over, maskPixel);
        }
    }
}

void subpixelComposit(PixelMap<wsRGBA>& under, wsRGBA over, PixelMap<uint8_t> const& mask) noexcept
{
    required_assert(mask.height >= under.height);
    required_assert((mask.width * 3) >= under.width);

    for (auto rowNr = 0; rowNr < under.height; rowNr++) {
        let maskRow = mask.at(rowNr);
        auto underRow = under.at(rowNr);
        for (auto maskColumnNr = 0, columnNr = 0; columnNr < static_cast<size_t>(underRow.width); columnNr++, maskColumnNr += 3) {
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

void desaturate(PixelMap<wsRGBA> &dst, float brightness) noexcept
{
    required_assert(brightness > 0.0 && brightness <= 1.0);
    let _brightness = static_cast<uint16_t>(brightness * 32768.0); 

    for (auto rowNr = 0; rowNr < dst.height; rowNr++) {
        auto dstRow = dst.at(rowNr);
        for (auto columnNr = 0; columnNr < dstRow.width; columnNr++) {
            auto &dstPixel = dstRow[columnNr];
            dstPixel.desaturate(_brightness);
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
    required_assert(image.width % 3 == 0);

    for (auto rowNr = 0; rowNr < image.height; rowNr++) {
        auto row = image.at(rowNr);
        for (auto columnNr = 0; columnNr < static_cast<size_t>(row.width); columnNr += 3) {
            std::swap(row[columnNr], row[columnNr + 2]);
        }
    }
}

}
