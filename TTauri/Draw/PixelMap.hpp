// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Path.hpp"
#include "TTauri/utils.hpp"
#include "TTauri/geometry.hpp"
#include "TTauri/Color.hpp"

#include <glm/glm.hpp>
#include <gsl/gsl>
#include <string>

#include <immintrin.h>


namespace TTauri::Draw {

enum class SubpixelOrientation {
    RedLeft,
    RedRight,
    Unknown
};

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
        size_t const offset = rect.offset.y * stride + rect.offset.x;
        size_t const count = (rect.extent.y - 1) * stride + rect.extent.x;

        if ((rect.offset.x + rect.extent.x > width) || (rect.offset.y + rect.extent.y > height)) {
            throw std::out_of_range("(rect.offset.x + rect.extent.x > width) || (rect.offset.y + rect.extent.y > height)");
        }
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
};

/*! Make the pixel around the border transparent.
 * But copy the color information from the neighbour pixel so that linear
 * interpolation near the border will work propertly.
 */
void add1PixelTransparentBorder(PixelMap<uint32_t> &pixelMap);

/*! Render path in subpixel mask.
 * Each uint32_t pixel is split into the three 10bit words. MSB->LSB: left-sub-pixel, mid-sub-pixel, right-sub-pixel.
 * The rendering will only increase the value on the pixels, so multiple pieces of text can
 * be rendered on top of the same pixels.
 */
void renderSubpixelMask(PixelMap<uint32_t> &mask, Path const &path);

/*! Render path directly into an image.
 */
void render(PixelMap<uint32_t> &pixels, Path const &path, Color_XYZ color, SubpixelOrientation subpixelOrientation=SubpixelOrientation::Unknown);

/*! Filter subpixels in PixelMap.
 * Each uint32_t pixel is split into the four bytes. MSB->LSB: left-sub-pixel, mid-sub-pixel, right-sub-pixel, color-index.
 * The pixels will be filtered so they wi
 */
void subpixelFiltering(PixelMap<uint32_t> &pixels, SubpixelOrientation subpixelOrientation);

/*! Composit colors from the color table based on the mask onto destination.
 */
void subpixelComposit(PixelMap<uint32_t>& destination, PixelMap<uint32_t> const& source, PixelMap<uint32_t> const& mask, Color_XYZ color);

/*! Composit colors from the color table based on the mask onto destination.
 * Mask should be subpixelFiltered before use.
 */
void subpixelComposit(PixelMap<uint32_t>& destination, PixelMap<uint32_t> const& mask, Color_XYZ color);

}
