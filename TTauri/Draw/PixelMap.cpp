// Copyright 2019 Pokitec
// All rights reserved.

#include "PixelMap.hpp"
#include "TTauri/Color.hpp"
#include <boost/endian/conversion.hpp>

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



void copyLinearToGamma(PixelMap<uint32_t>& dst, PixelMap<wsRGBApm> const& src)
{
    assert(dst.width >= src.width);
    assert(dst.height >= src.height);

    for (size_t rowNr = 0; rowNr < src.height; rowNr++) {
        let srcRow = src.at(rowNr);
        let dstRow = dst.at(rowNr);
        for (size_t columnNr = 0; columnNr < src.width; columnNr++) {
            dstRow[columnNr] = boost::endian::native_to_big(srcRow[columnNr].to_sRGBApm_u32());
        }
    }
}



}
