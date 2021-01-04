// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "pixel_map.hpp"

namespace tt {

template<int KERNEL_SIZE, typename KERNEL>
inline void horizontalFilterRow(pixel_row<uint8_t> row, KERNEL kernel) noexcept
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
inline void horizontalFilter(pixel_map<T>& pixels, KERNEL kernel) noexcept
{
    for (int rowNr = 0; rowNr < pixels.height; rowNr++) {
        auto row = pixels.at(rowNr);
        horizontalFilterRow<KERNEL_SIZE>(row, kernel);
    }
}

template<typename T>
inline void fill(pixel_map<T> &dst) noexcept
{
    for (int rowNr = 0; rowNr < dst.height(); rowNr++) {
        auto row = dst.at(rowNr);
        void *data = row.data();
        memset(data, 0, row.width() * sizeof(T));
    }
}

template<typename T>
inline void fill(pixel_map<T> &dst, T color) noexcept
{
    for (int rowNr = 0; rowNr < dst.height; rowNr++) {
        auto row = dst.at(rowNr);
        for (int columnNr = 0; columnNr < row.width; columnNr++) {
            row[columnNr] = color;
        }
    }
}

template<typename T>
inline void rotate90(pixel_map<T> &dst, pixel_map<T> const &src) noexcept
{
    assert(dst.width() >= src.height());
    assert(dst.height() >= src.width());

    for (int rowNr = 0; rowNr < src.height(); rowNr++) {
        ttlet row = src.at(rowNr);
        ttlet dstColumnNr = src.height() - rowNr - 1;
        auto dstRowNr = 0;
        for (int columnNr = 0; columnNr < row.width(); columnNr++) {
            dst[dstRowNr++][dstColumnNr] = row[columnNr];
        }
    }
}

template<typename T>
inline void rotate270(pixel_map<T> &dst, pixel_map<T> const &src) noexcept
{
    assert(dst.width() >= src.height());
    assert(dst.height() >= src.width());

    for (int rowNr = 0; rowNr < src.height(); rowNr++) {
        ttlet row = src.at(rowNr);
        ttlet dstColumnNr = rowNr;
        auto dstRowNr = row.width() - 1;
        for (int columnNr = 0; columnNr < row.width(); columnNr++) {
            dst[dstRowNr--][dstColumnNr] = row[columnNr];
        }
    }
}

template<typename T>
inline void makeTransparentBorder(pixel_map<T> & pixel_map) noexcept
{
    auto topBorder = pixel_map.at(0);
    ttlet topRow = pixel_map.at(1);
    ttlet bottomRow = pixel_map.at(pixel_map.height() - 2);
    auto bottomBorder = pixel_map.at(pixel_map.height() - 1);
    for (auto x = 1; x < pixel_map.width() - 1; x++) {
        topBorder[x] = makeTransparent(topRow[x]);
        bottomBorder[x] = makeTransparent(bottomRow[x]);
    }

    ttlet rightBorderY = pixel_map.width() - 1;
    ttlet rightY = pixel_map.width() - 2;
    for (auto y = 1; y < pixel_map.height() - 1; y++) {
        auto row = pixel_map[y];
        row[0] = makeTransparent(row[1]);
        row[rightBorderY] = makeTransparent(row[rightY]);
    }

    pixel_map[0][0] = makeTransparent(pixel_map[1][1]);
    pixel_map[0][pixel_map.width() - 1] = makeTransparent(pixel_map[1][pixel_map.width() - 2]);
    pixel_map[pixel_map.height() - 1][0] = makeTransparent(pixel_map[pixel_map.height() - 2][1]);
    pixel_map[pixel_map.height() - 1][pixel_map.width() - 1] =
        makeTransparent(pixel_map[pixel_map.height() - 2][pixel_map.width() - 2]);
}

}

