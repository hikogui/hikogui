// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/geometry.hpp"
#include "TTauri/required.hpp"
#include <glm/glm.hpp>
#include <gsl/gsl>
#include <string>
#include <algorithm>

namespace TTauri {
struct wsRGBA;
}

namespace TTauri::Draw {

template <typename T>
struct PixelRow {
    T * const pixels;
    size_t const width;

    T const* data() const {
        return pixels;
    }

    T * data() {
        return pixels;
    }

    T const &operator[](size_t columnNr) const {
        return pixels[columnNr];
    }

    T &operator[](size_t columnNr) {
        return pixels[columnNr];
    }

    T const &at(size_t columnNr) const {
        if (columnNr >= width) {
            throw std::out_of_range("columnNr >= height");
        }
        return pixels[columnNr];
    }

    T &at(size_t columnNr) {
        if (columnNr >= width) {
            throw std::out_of_range("columnNr >= height");
        }
        return pixels[columnNr];
    }
};

template <typename T>
struct PixelMap {
    T *pixels;
    size_t width;
    size_t height;
    size_t stride;
    bool selfAllocated = false;

    PixelMap() : pixels(nullptr), width(0), height(0), stride(0) {}

    PixelMap(T *pixels, size_t width, size_t height, size_t stride) : pixels(pixels), width(width), height(height), stride(stride) {
        if (pixels) {
            required_assert(stride >= width);
            required_assert(width > 0);
            required_assert(height > 0);
        } else {
            required_assert(width == 0);
            required_assert(height == 0);
        }
    } 

    PixelMap(size_t width, size_t height) : pixels(new T[width * height]), width(width), height(height), stride(width), selfAllocated(true) {
        if (pixels) {
            required_assert(stride >= width);
            required_assert(width > 0);
            required_assert(height > 0);
        } else {
            required_assert(width == 0);
            required_assert(height == 0);
        }
    }

    PixelMap(u64extent2 extent) : PixelMap(extent.width(), extent.height()) {}
    PixelMap(T *pixels, size_t width, size_t height) : PixelMap(pixels, width, height, width) {}
    PixelMap(T *pixels, u64extent2 extent) : PixelMap(pixels, extent.width(), extent.height()) {}
    PixelMap(T *pixels, u64extent2 extent, size_t stride) : PixelMap(pixels, extent.width(), extent.height(), stride) {}

    ~PixelMap() {
        if (selfAllocated) {
            delete[] pixels;
        }
    }

    /*! Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
     */
    PixelMap(PixelMap const &other) = delete;
    PixelMap(PixelMap &&other) : pixels(other.pixels), width(other.width), height(other.height), stride(other.stride), selfAllocated(other.selfAllocated) {
        other.selfAllocated = false;
    }

    operator bool() const {
        return pixels;
    }

    /*! Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
    */
    PixelMap &operator=(PixelMap const &other) = delete;
    PixelMap &operator=(PixelMap &&other) {
        if (selfAllocated) {
            delete[] pixels;
        }
        pixels = other.pixels;
        width = other.width;
        height = other.height;
        stride = other.stride;
        selfAllocated = other.selfAllocated;
        other.selfAllocated = false;
        return *this;
    }

    PixelMap<T> submap(u64rect2 rect) const {
        required_assert(rect.extent.x > 0 && rect.extent.y > 0);

        if ((rect.offset.x + rect.extent.x > width) || (rect.offset.y + rect.extent.y > height)) {
            throw std::out_of_range("(rect.offset.x + rect.extent.x > width) || (rect.offset.y + rect.extent.y > height)");
        }
        
        size_t const offset = rect.offset.y * stride + rect.offset.x;

        if (rect.extent.y == 0 || rect.extent.x == 0) {
            // Image of zero width or height needs zero pixels returned.
            return { };
        }

        return { pixels + offset, rect.extent.x, rect.extent.y, stride };
    }
    
    PixelMap<T> submap(size_t const x, size_t const y, size_t const width, size_t const height) const {
        return submap({{x, y}, {width, height}});
    }

    PixelRow<T> const operator[](size_t const rowNr) const {
        return { pixels + (rowNr * stride), width };
    }

    PixelRow<T> operator[](size_t const rowNr) {
        return { pixels + (rowNr * stride), width };
    }

    PixelRow<T> const at(const size_t rowNr) const {
        if (rowNr >= height) {
            throw std::out_of_range("rowNr >= height");
        }
        return (*this)[rowNr];
    }

    PixelRow<T> at(const size_t rowNr) {
        if (rowNr >= height) {
            throw std::out_of_range("rowNr >= height");
        }
        return (*this)[rowNr];
    }

    std::vector<void *> rowPointers() {
        std::vector<void *> r;
        r.reserve(height);

        for (size_t row = 0; row < height; row++) {
            void *ptr = at(row).data();
            r.push_back(ptr);
        }

        return r;
    }

};

template<int KERNEL_SIZE, typename KERNEL>
void horizontalFilterRow(PixelRow<uint8_t> row, KERNEL kernel);

template<int KERNEL_SIZE, typename T, typename KERNEL>
void horizontalFilter(PixelMap<T>& pixels, KERNEL kernel);

/*! Clear the pixels of this (sub)image.
 */
template<typename T>
void fill(PixelMap<T> &dst);

/*! Fill with color.
 */
template<typename T>
void fill(PixelMap<T> &dst, T color);

/*! Rotate an image 90 degrees counter-clockwise.
 */
template<typename T>
void rotate90(PixelMap<T> &dst, PixelMap<T> const &src);

/*! Rotate an image 270 degrees counter-clockwise.
 */
template<typename T>
void rotate270(PixelMap<T> &dst, PixelMap<T> const &src);

/*! Merge two image by applying std::max on each pixel.
 */
void mergeMaximum(PixelMap<uint8_t> &dst, PixelMap<uint8_t> const &src);

/*! Make the pixel around the border transparent.
 * But copy the color information from the neighbour pixel so that linear
 * interpolation near the border will work propertly.
 */
void addTransparentBorder(PixelMap<uint32_t>& pixelMap);

/*! Copy a image with linear 16bit-per-color-component to a
 * gamma corrected 8bit-per-color-component image.
 */
void fill(PixelMap<uint32_t>& dst, PixelMap<wsRGBA> const& src);

/*! Composit the image `over` onto the image `under`.
 */
void composit(PixelMap<wsRGBA> &under, PixelMap<wsRGBA> const &over);

/*! Composit the color `over` onto the image `under` based on the pixel mask.
 */
void composit(PixelMap<wsRGBA>& under, wsRGBA over, PixelMap<uint8_t> const& mask);

/*! Desaturate an image.
 * \param brightness The image colours are multiplied by the brightness.
 */
void desaturate(PixelMap<wsRGBA> &dst, float brightness=1.0);

/*! Composit the color `over` onto the image `under` based on the subpixel mask.
 * Mask should be passed to subpixelFilter() before use.
 */
void subpixelComposit(PixelMap<wsRGBA>& under, wsRGBA over, PixelMap<uint8_t> const& mask);

/*! Execute a slight horiontal blur filter to reduce colour fringes with subpixel compositing.
 */
void subpixelFilter(PixelMap<uint8_t> &image);

/*! Swap R and B values of each RGB pixel.
 */
void subpixelFlip(PixelMap<uint8_t> &image);

}
