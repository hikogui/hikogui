// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/all.hpp"

#include <glm/glm.hpp>
#include <gsl/gsl>
#include <string>
#include <algorithm>

namespace TTauri::Draw {

template <typename T>
struct PixelMap {
    bool selfAllocated = false;
    gsl::span<T> pixels;
    size_t width;
    size_t height;

    //! Stride in number of pixels; width of the original image.
    size_t stride;

    PixelMap() : pixels({}), width(0), height(0), stride(0) {}
    PixelMap(size_t width, size_t height) : width(width), height(height), stride(width), selfAllocated(true) {
        T *data = new T[width * height];
        pixels = {data, boost::numeric_cast<ptrdiff_t>(width * height)};
    }
    PixelMap(glm::u64vec2 extent) : width(extent.x), height(extent.y), stride(extent.x), selfAllocated(true) {
        T* data = new T[width * height];
        pixels = { data, boost::numeric_cast<ptrdiff_t>(width * height) };
    }

    PixelMap(gsl::span<T> pixels, size_t width, size_t height) : pixels(pixels), width(width), height(height), stride(width) {}
    PixelMap(gsl::span<T> pixels, size_t width, size_t height, size_t stride) : pixels(pixels), width(width), height(height), stride(stride) {}
    PixelMap(gsl::span<T> pixels, glm::u64vec2 extent) : pixels(pixels), width(extent.x), height(extent.y), stride(extent.x) {}
    PixelMap(gsl::span<T> pixels, glm::u64vec2 extent, size_t stride) : pixels(pixels), width(extent.x), height(extent.y), stride(stride) {}

    ~PixelMap() {
        if (selfAllocated) {
            delete pixels.data();
        }
    }

    PixelMap<T> submap(u64rect2 rect) const {
        if ((rect.offset.x + rect.extent.x > width) || (rect.offset.y + rect.extent.y > height)) {
            throw std::out_of_range("(rect.offset.x + rect.extent.x > width) || (rect.offset.y + rect.extent.y > height)");
        }
        
        size_t const offset = rect.offset.y * stride + rect.offset.x;

        if (rect.extent.y == 0 || rect.extent.x == 0) {
            // Image of zero width or height needs zero pixels returned.
            return { pixels.subspan(offset, 0), rect.extent.x, rect.extent.y, stride };
        }

        size_t const count = (rect.extent.y - 1) * stride + rect.extent.x;

        return { pixels.subspan(offset, count), rect.extent.x, rect.extent.y, stride };
    }
    
    PixelMap<T> submap(size_t const x, size_t const y, size_t const width, size_t const height) const {
        return submap({{x, y}, {width, height}});
    }

    gsl::span<T> operator[](size_t const rowNr) const {
        return pixels.subspan(rowNr * stride, width);
    }

    gsl::span<T> at(const size_t rowNr) const {
        if (rowNr >= height) {
            throw std::out_of_range("rowNr >= height");
        }
        return (*this)[rowNr];
    }

    std::vector<void *> rowPointers() const {
        std::vector<void *> r;
        r.reserve(height);

        for (size_t row = 0; row < height; row++) {
            void *ptr = &(at(row).at(0));
            r.push_back(ptr);
        }

        return r;
    }

    /*! Clear the pixels of this (sub)image.
     */
    void clear() {
        for (size_t rowNr = 0; rowNr < height; rowNr++) {
            let row = this->at(rowNr);
            void *data = row.data();
            memset(data, 0, row.size_bytes());
        }
    }

    /*! Fill with color.
     */
    void fill(T color) {
        for (size_t rowNr = 0; rowNr < height; rowNr++) {
            let row = this->at(rowNr);
            for (size_t columnNr = 0; columnNr < width; columnNr++) {
                row[columnNr] = color;
            }
        }
    }
};

/*! Make the pixel around the border transparent.
 * But copy the color information from the neighbour pixel so that linear
 * interpolation near the border will work propertly.
 */
void add1PixelTransparentBorder(PixelMap<uint32_t> &pixelMap);

/*! Copy a image with linear 16bit-per-color-component to a
 * gamma corrected 8bit-per-color-component image.
 */
void copyLinearToGamma(PixelMap<uint32_t>& dst, PixelMap<wsRGBApm> const& src);

template<int N, typename Kernel>
void horizontalFilterRow(gsl::span<uint8_t> row, Kernel kernel) {
    constexpr auto halfN = N/2;

    uint64_t values = 0;
    ptrdiff_t x;

    // Start beyond the left pixel. Then lookahead upto
    // the point we can start the kernel.
    let leftEdgeValue = row[0];
    for (x = -N; x < 0; x++) {
        values <<= 8;

        let lookAheadX = x + halfN;
        if (lookAheadX < 0) {
            values |= leftEdgeValue;
        } else {
            values |= row[lookAheadX];
        }
    }

    // Execute the kernel on all the pixels upto the right edge.
    // The values are still looked up ahead.
    let lastX = row.size() - halfN;
    for (; x < lastX; x++) {
        values <<= 8;
        values |= row[x + halfN];

        row[x] = kernel(values);
    }

    // Finish up to the right edge.
    let rightEdgeValue = row[row.size() - 1];
    for (; x < row.size(); x++) {
        values <<= 8;
        values |= rightEdgeValue;

        row[x] = kernel(values);
    }
}

template<int N, typename T, typename Kernel>
void horizontalFilter(PixelMap<T> &pixels, Kernel kernel) {
    for (size_t rowNr = 0; rowNr < pixels.height; rowNr++) {
        auto row = pixels.at(rowNr);
        horizontalFilterRow<N>(row, kernel);
    }
}


}
