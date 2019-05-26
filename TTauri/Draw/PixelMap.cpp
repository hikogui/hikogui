// Copyright 2019 Pokitec
// All rights reserved.

#include "PixelMap.hpp"

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





}
