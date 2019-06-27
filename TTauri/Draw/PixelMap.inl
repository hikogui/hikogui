// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PixelMap.hpp"

namespace TTauri::Draw {

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

template<typename T>
inline void fill(PixelMap<T> &dst)
{
    for (size_t rowNr = 0; rowNr < dst.height; rowNr++) {
        auto row = dst.at(rowNr);
        void *data = row.data();
        memset(data, 0, row.width * sizeof(T));
    }
}

template<typename T>
inline void fill(PixelMap<T> &dst, T color)
{
    for (size_t rowNr = 0; rowNr < dst.height; rowNr++) {
        auto row = dst.at(rowNr);
        for (size_t columnNr = 0; columnNr < row.width; columnNr++) {
            row[columnNr] = color;
        }
    }
}

template<typename T>
inline void rotate90(PixelMap<T> &dst, PixelMap<T> const &src)
{
    assert(dst.width >= src.height);
    assert(dst.height >= src.width);

    for (size_t rowNr = 0; rowNr < src.height; rowNr++) {
        let row = src.at(rowNr);
        size_t dstColumnNr = src.height - rowNr - 1;
        size_t dstRowNr = 0;
        for (size_t columnNr = 0; columnNr < row.width; columnNr++) {
            dst[dstRowNr++][dstColumnNr] = row[columnNr];
        }
    }
}

template<typename T>
inline void rotate270(PixelMap<T> &dst, PixelMap<T> const &src)
{
    assert(dst.width >= src.height);
    assert(dst.height >= src.width);

    for (size_t rowNr = 0; rowNr < src.height; rowNr++) {
        let row = src.at(rowNr);
        size_t dstColumnNr = rowNr;
        size_t dstRowNr = row.width - 1;
        for (size_t columnNr = 0; columnNr < row.width; columnNr++) {
            dst[dstRowNr--][dstColumnNr] = row[columnNr];
        }
    }
}

}

