// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/geometry.hpp"
#include "TTauri/Required/required.hpp"
#include <glm/glm.hpp>
#include <gsl/gsl>
#include <string>
#include <algorithm>
#include <vector>

namespace TTauri {
struct wsRGBA;
}

namespace TTauri::Draw {

template <typename T>
struct PixelRow {
    T *pixels;
    int width;

    T const *data() const noexcept {
        return pixels;
    }

    T *data() noexcept {
        return pixels;
    }

    T const &operator[](int columnNr) const noexcept {
        return pixels[columnNr];
    }

    T &operator[](int columnNr) noexcept {
        return pixels[columnNr];
    }

    T const &at(int columnNr) const noexcept {
        required_assert(columnNr >= 0 && columnNr < width);
        return pixels[columnNr];
    }

    T &at(int columnNr) noexcept {
        required_assert(columnNr >= 0 && columnNr < width);
        return pixels[columnNr];
    }
};

template <typename T>
struct PixelMap {
    T *pixels;
    int width;
    int height;
    int stride;
    bool selfAllocated = false;

    PixelMap() noexcept : pixels(nullptr), width(0), height(0), stride(0) {}

    PixelMap(T *pixels, int width, int height, int stride) noexcept : pixels(pixels), width(width), height(height), stride(stride) {
        if (pixels) {
            required_assert(stride >= width);
            required_assert(width > 0);
            required_assert(height > 0);
        } else {
            required_assert(width == 0);
            required_assert(height == 0);
        }
    } 

    gsl_suppress(r.11)
    PixelMap(int width, int height) noexcept : pixels(new T[width * height]), width(width), height(height), stride(width), selfAllocated(true) {
        if (pixels) {
            required_assert(stride >= width);
            required_assert(width > 0);
            required_assert(height > 0);
        } else {
            required_assert(width == 0);
            required_assert(height == 0);
        }
    }

    PixelMap(iextent2 extent) noexcept : PixelMap(extent.width(), extent.height()) {}
    PixelMap(T *pixels, int width, int height) noexcept : PixelMap(pixels, width, height, width) {}
    PixelMap(T *pixels, iextent2 extent) noexcept : PixelMap(pixels, extent.width(), extent.height()) {}
    PixelMap(T *pixels, iextent2 extent, int stride) noexcept : PixelMap(pixels, extent.width(), extent.height(), stride) {}

    gsl_suppress2(r.11,i.11)
    ~PixelMap() {
        if (selfAllocated) {
            delete[] pixels;
        }
    }

    /*! Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
     */
    PixelMap(PixelMap const &other) = delete;
    PixelMap(PixelMap &&other) noexcept : pixels(other.pixels), width(other.width), height(other.height), stride(other.stride), selfAllocated(other.selfAllocated) {
        other.selfAllocated = false;
    }

    operator bool() const noexcept {
        return pixels;
    }

    /*! Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
    */
    PixelMap &operator=(PixelMap const &other) = delete;

    gsl_suppress2(r.11,i.11)
    PixelMap &operator=(PixelMap &&other) noexcept {
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

    PixelMap<T> submap(irect2 rect) const noexcept {
        required_assert(
            (rect.offset.x >= 0) &&
            (rect.offset.y >= 0) &&
            (rect.extent.width() >= 0) &&
            (rect.extent.height() >= 0)
        );
        required_assert(
            (rect.offset.x + rect.extent.width() <= width) &&
            (rect.offset.y + rect.extent.height() <= height)
        );

        let offset = rect.offset.y * stride + rect.offset.x;

        if (rect.extent.width() == 0 || rect.extent.height() == 0) {
            // Image of zero width or height needs zero pixels returned.
            return { };
        } else {
            return { pixels + offset, rect.extent.x, rect.extent.y, stride };
        }
    }
    
    PixelMap<T> submap(int const x, int const y, int const width, int const height) const noexcept {
        return submap(irect2{{x, y}, {width, height}});
    }

    PixelRow<T> const operator[](int const rowNr) const noexcept {
        return { pixels + (rowNr * stride), width };
    }

    PixelRow<T> operator[](int const rowNr) noexcept {
        return { pixels + (rowNr * stride), width };
    }

    PixelRow<T> const at(int const rowNr) const noexcept {
        required_assert(rowNr < height);
        return (*this)[rowNr];
    }

    PixelRow<T> at(int const rowNr) noexcept {
        required_assert(rowNr < height);
        return (*this)[rowNr];
    }

    
    std::vector<void *> rowPointers() noexcept {
        std::vector<void *> r;
        r.reserve(height);

        for (auto row = 0; row < height; row++) {
            void *ptr = at(row).data();
            r.push_back(ptr);
        }

        return r;
    }

};

template<int KERNEL_SIZE, typename KERNEL>
void horizontalFilterRow(PixelRow<uint8_t> row, KERNEL kernel) noexcept;

template<int KERNEL_SIZE, typename T, typename KERNEL>
void horizontalFilter(PixelMap<T>& pixels, KERNEL kernel) noexcept;

/*! Clear the pixels of this (sub)image.
 */
template<typename T>
void fill(PixelMap<T> &dst) noexcept;

/*! Fill with color.
 */
template<typename T>
void fill(PixelMap<T> &dst, T color) noexcept;

/*! Rotate an image 90 degrees counter-clockwise.
 */
template<typename T>
void rotate90(PixelMap<T> &dst, PixelMap<T> const &src) noexcept;

/*! Rotate an image 270 degrees counter-clockwise.
 */
template<typename T>
void rotate270(PixelMap<T> &dst, PixelMap<T> const &src) noexcept;

/*! Merge two image by applying std::max on each pixel.
 */
void mergeMaximum(PixelMap<uint8_t> &dst, PixelMap<uint8_t> const &src) noexcept;

/*! Make the pixel around the border transparent.
 * But copy the color information from the neighbour pixel so that linear
 * interpolation near the border will work propertly.
 */
void addTransparentBorder(PixelMap<uint32_t>& pixelMap) noexcept;

/*! Copy a image with linear 16bit-per-color-component to a
 * gamma corrected 8bit-per-color-component image.
 */
void fill(PixelMap<uint32_t>& dst, PixelMap<wsRGBA> const& src) noexcept;

/*! Composit the image `over` onto the image `under`.
 */
void composit(PixelMap<wsRGBA> &under, PixelMap<wsRGBA> const &over) noexcept;

/*! Composit the color `over` onto the image `under` based on the pixel mask.
 */
void composit(PixelMap<wsRGBA>& under, wsRGBA over, PixelMap<uint8_t> const& mask) noexcept;

/*! Desaturate an image.
 * \param brightness The image colours are multiplied by the brightness.
 */
void desaturate(PixelMap<wsRGBA> &dst, float brightness=1.0) noexcept;

/*! Composit the color `over` onto the image `under` based on the subpixel mask.
 * Mask should be passed to subpixelFilter() before use.
 */
void subpixelComposit(PixelMap<wsRGBA>& under, wsRGBA over, PixelMap<uint8_t> const& mask) noexcept;

/*! Execute a slight horiontal blur filter to reduce colour fringes with subpixel compositing.
 */
void subpixelFilter(PixelMap<uint8_t> &image) noexcept;

/*! Swap R and B values of each RGB pixel.
 */
void subpixelFlip(PixelMap<uint8_t> &image) noexcept;

}
