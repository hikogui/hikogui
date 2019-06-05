// Copyright 2019 Pokitec
// All rights reserved.

#include "PixelMap.hpp"
#include "TTauri/all.hpp"

namespace TTauri::Draw {

void add1PixelTransparentBorder(PixelMap<uint32_t> &pixelMap)
{
    uint8_t const u8invisibleMask[4] = { 0xff, 0xff, 0xff, 0 };
    uint32_t u32invisibleMask;
    std::memcpy(&u32invisibleMask, u8invisibleMask, sizeof(u32invisibleMask));

    let topBorder = pixelMap.at(0);
    let topRow = pixelMap.at(1);
    let bottomRow = pixelMap.at(pixelMap.height - 2);
    let bottomBorder = pixelMap.at(pixelMap.height - 1);
    for (size_t x = 1; x < pixelMap.width - 1; x++) {
        topBorder[x] = topRow[x] & u32invisibleMask;
        bottomBorder[x] = bottomRow[x] & u32invisibleMask;
    }

    let rightBorderY = pixelMap.width - 1;
    let rightY = pixelMap.width - 2;
    for (size_t y = 1; y < pixelMap.height - 1; y++) {
        let row = pixelMap[y];
        row[0] = row[1] & u32invisibleMask;
        row[rightBorderY] = row[rightY] & u32invisibleMask;
    }

    pixelMap[0][0] = pixelMap[1][1] & u32invisibleMask;
    pixelMap[0][pixelMap.width - 1] = pixelMap[1][pixelMap.width - 2] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][0] = pixelMap[pixelMap.height - 2][1] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][pixelMap.width - 1] = pixelMap[pixelMap.height - 2][pixelMap.width - 2] & u32invisibleMask;
}

const auto linearToGamma = generate_array<uint8_t,257>([](auto i) {
    let u = i / 255.0f;
    
    if (u <= 0.0031308f) {
        return (u * 12.92f) * 255.0f;
    } else {
        return (std::pow(u * 1.055f, 1.0f / 2.4f) - 0.055f) * 255.0f;
    }
});

static uint8_t linearToGammaInterpolate(uint16_t input)
{
    let index = input >> 8;
    let low = linearToGamma[index];
    let high = linearToGamma[index + 1];
    let diff = high - low;

    let scaledDiff = diff * (input & 0xff);
    return low + (scaledDiff >> 8); 
}

void copyLinearToGamma(PixelMap<uint32_t>& dst, PixelMap<uint64_t> const& src)
{
    assert(dst.width >= src.width);
    assert(dst.height >= src.height);

    for (size_t rowNr = 0; rowNr < src.height; rowNr++) {
        let srcRow = src.at(rowNr);
        let dstRow = dst.at(rowNr);
        for (size_t columnNr = 0; columnNr < src.width; columnNr++) {
            let valueLinear = srcRow[columnNr];

            let valueLinearRed = (valueLinear >> 48) & 0xffff;
            let valueLinearGreen = (valueLinear >> 32) & 0xffff;
            let valueLinearBlue = (valueLinear >> 16) & 0xffff;
            let valueLinearAlpha = valueLinear & 0xffff;

            let valueGammaRed = linearToGammaInterpolate(valueLinearRed);
            let valueGammaGreen = linearToGammaInterpolate(valueLinearGreen);
            let valueGammaBlue = linearToGammaInterpolate(valueLinearBlue);
            let valueGammaAlpha = valueLinearAlpha >> 8;

            uint32_t valueGamma = (
                (valueGammaRed << 24) |
                (valueGammaGreen << 16) |
                (valueGammaBlue << 8) |
                valueGammaAlpha
            );

            dstRow[columnNr] = boost::endian::native_to_big(valueGamma);
        }
    }
}



}
