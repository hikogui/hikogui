// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "pixel_map.hpp"

namespace hi::inline v1 {

template<int KERNEL_SIZE, typename KERNEL>
inline void horizontalFilterRow(pixel_row<uint8_t> row, KERNEL kernel) noexcept
{
    constexpr auto LOOK_AHEAD_SIZE = KERNEL_SIZE / 2;

    uint64_t values = 0;
    auto x = -KERNEL_SIZE;

    // Start beyond the left pixel. Then lookahead upto
    // the point we can start the kernel.
    hilet leftEdgeValue = row[0];
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
    hilet lastX = row.width - LOOK_AHEAD_SIZE;
    for (; x < lastX; x++) {
        values <<= 8;
        values |= row[LOOK_AHEAD_SIZE + x];

        row[x] = kernel(values);
    }

    // Finish up to the right edge.
    hilet rightEdgeValue = row[row.width - 1];
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
        hilet row = src.at(rowNr);
        hilet dstColumnNr = src.height() - rowNr - 1;
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
        hilet row = src.at(rowNr);
        hilet dstColumnNr = rowNr;
        auto dstRowNr = row.width() - 1;
        for (int columnNr = 0; columnNr < row.width(); columnNr++) {
            dst[dstRowNr--][dstColumnNr] = row[columnNr];
        }
    }
}

}

