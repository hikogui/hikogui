// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PixelMap.hpp"

namespace tt {

template<int KERNEL_SIZE, typename KERNEL>
inline void horizontalFilterRow(PixelRow<uint8_t> row, KERNEL kernel) noexcept
{
    constexpr auto LOOK_AHEAD_SIZE = KERNEL_SIZE / 2;

    uint64_t values = 0;
    auto x = -KERNEL_SIZE;

    // Start beyond the left pixel. Then lookahead upto
    // the point we can start the kernel.
    ttlet leftEdgeValue = row[0];
    for (; x < 0; x++) {
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
    ttlet lastX = row.width - LOOK_AHEAD_SIZE;
    for (; x < lastX; x++) {
        values <<= 8;
        values |= row[LOOK_AHEAD_SIZE + x];

        row[x] = kernel(values);
    }

    // Finish up to the right edge.
    ttlet rightEdgeValue = row[row.width - 1];
    for (; x < row.width; x++) {
        values <<= 8;
        values |= rightEdgeValue;

        row[x] = kernel(values);
    }
}

template<int KERNEL_SIZE, typename T, typename KERNEL>
inline void horizontalFilter(PixelMap<T>& pixels, KERNEL kernel) noexcept
{
    for (int rowNr = 0; rowNr < pixels.height; rowNr++) {
        auto row = pixels.at(rowNr);
        horizontalFilterRow<KERNEL_SIZE>(row, kernel);
    }
}

template<typename T>
inline void fill(PixelMap<T> &dst) noexcept
{
    for (int rowNr = 0; rowNr < dst.height; rowNr++) {
        auto row = dst.at(rowNr);
        void *data = row.data();
        memset(data, 0, row.width * sizeof(T));
    }
}

template<typename T>
inline void fill(PixelMap<T> &dst, T color) noexcept
{
    for (int rowNr = 0; rowNr < dst.height; rowNr++) {
        auto row = dst.at(rowNr);
        for (int columnNr = 0; columnNr < row.width; columnNr++) {
            row[columnNr] = color;
        }
    }
}

template<typename T>
inline void rotate90(PixelMap<T> &dst, PixelMap<T> const &src) noexcept
{
    assert(dst.width >= src.height);
    assert(dst.height >= src.width);

    for (int rowNr = 0; rowNr < src.height; rowNr++) {
        ttlet row = src.at(rowNr);
        ttlet dstColumnNr = src.height - rowNr - 1;
        auto dstRowNr = 0;
        for (int columnNr = 0; columnNr < row.width; columnNr++) {
            dst[dstRowNr++][dstColumnNr] = row[columnNr];
        }
    }
}

template<typename T>
inline void rotate270(PixelMap<T> &dst, PixelMap<T> const &src) noexcept
{
    assert(dst.width >= src.height);
    assert(dst.height >= src.width);

    for (int rowNr = 0; rowNr < src.height; rowNr++) {
        ttlet row = src.at(rowNr);
        ttlet dstColumnNr = rowNr;
        auto dstRowNr = row.width - 1;
        for (int columnNr = 0; columnNr < row.width; columnNr++) {
            dst[dstRowNr--][dstColumnNr] = row[columnNr];
        }
    }
}

template<typename T>
inline void makeTransparentBorder(PixelMap<T> & pixelMap) noexcept
{
    auto topBorder = pixelMap.at(0);
    ttlet topRow = pixelMap.at(1);
    ttlet bottomRow = pixelMap.at(pixelMap.height - 2);
    auto bottomBorder = pixelMap.at(pixelMap.height - 1);
    for (auto x = 1; x < pixelMap.width - 1; x++) {
        topBorder[x] = makeTransparent(topRow[x]);
        bottomBorder[x] = makeTransparent(bottomRow[x]);
    }

    ttlet rightBorderY = pixelMap.width - 1;
    ttlet rightY = pixelMap.width - 2;
    for (auto y = 1; y < pixelMap.height - 1; y++) {
        auto row = pixelMap[y];
        row[0] = makeTransparent(row[1]);
        row[rightBorderY] = makeTransparent(row[rightY]);
    }

    pixelMap[0][0] = makeTransparent(pixelMap[1][1]);
    pixelMap[0][pixelMap.width - 1] = makeTransparent(pixelMap[1][pixelMap.width - 2]);
    pixelMap[pixelMap.height - 1][0] = makeTransparent(pixelMap[pixelMap.height - 2][1]);
    pixelMap[pixelMap.height - 1][pixelMap.width - 1] = makeTransparent(pixelMap[pixelMap.height - 2][pixelMap.width - 2]);
}

}

