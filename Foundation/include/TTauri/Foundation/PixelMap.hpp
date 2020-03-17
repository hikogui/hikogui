// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/irect.hpp"
#include <gsl/gsl>
#include <string>
#include <algorithm>
#include <vector>

namespace TTauri {
struct wsRGBA;
}

namespace TTauri {

/** A row of pixels.
 */
template <typename T>
struct PixelRow {
    /** Pointer to an array of pixels.
     */
    T *pixels;

    /** Number of pixels in the row.
     */
    int width;

    /** Get a pointer to the pixel data.
     */
    T const *data() const noexcept {
        return pixels;
    }

    /** Get a pointer to the pixel data.
     */
    T *data() noexcept {
        return pixels;
    }

    /** Get a access to a pixel in the row.
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    T const &operator[](int columnNr) const noexcept {
        return pixels[columnNr];
    }

    /** Get a access to a pixel in the row.
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    T &operator[](int columnNr) noexcept {
        return pixels[columnNr];
    }

    /** Get a access to a pixel in the row.
     * This function does bound checking.
     *
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    T const &at(int columnNr) const noexcept {
        ttauri_assert(columnNr >= 0 && columnNr < width);
        return pixels[columnNr];
    }

    /** Get a access to a pixel in the row.
     * This function does bound checking.
     *
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    T &at(int columnNr) noexcept {
        ttauri_assert(columnNr >= 0 && columnNr < width);
        return pixels[columnNr];
    }
};

/** A 2D canvas of pixels.
 * This class may either allocate its own memory, or gives access
 * to memory allocated by another API, such as a Vulkan texture.
 */
template <typename T>
struct PixelMap {
    /** Pointer to a 2D canvas of pixels.
     */
    T *pixels;

    /** Number of horizontal pixels.
     */
    int width;

    /** Number of vertical pixels.
     */
    int height;

    /** Number of pixel element until the next row.
     * This is used when the alignment of each row is different from the width of the canvas.
     */
    int stride;

    /** True if the memory was allocated by this class, false if the canvas was received from another API.
     */
    bool selfAllocated = false;

    /** Construct an empty pixel-map.
     */
    PixelMap() noexcept : pixels(nullptr), width(0), height(0), stride(0) {}

    /** Construct an pixel-map from memory received from an API.
     * @param pixel A pointer to pixels received from the API.
     * @param width The width of the image.
     * @param height The height of the image.
     * @param stride Number of pixel elements until the next row.
     */
    PixelMap(T *pixels, int width, int height, int stride) noexcept : pixels(pixels), width(width), height(height), stride(stride) {
        if (pixels) {
            ttauri_assert(stride >= width);
            ttauri_assert(width > 0);
            ttauri_assert(height > 0);
        } else {
            ttauri_assert(width == 0);
            ttauri_assert(height == 0);
        }
    } 

    /** Construct an pixel-map.
     * This constructor will allocate its own memory.
     *
     * @param width The width of the image.
     * @param height The height of the image.
     */
    gsl_suppress(r.11)
    PixelMap(int width, int height) noexcept : pixels(new T[width * height]), width(width), height(height), stride(width), selfAllocated(true) {
        if (pixels) {
            ttauri_assert(stride >= width);
            ttauri_assert(width > 0);
            ttauri_assert(height > 0);
        } else {
            ttauri_assert(width == 0);
            ttauri_assert(height == 0);
        }
    }

    /** Construct an pixel-map.
     * This constructor will allocate its own memory.
     *
     * @param extent The width and height of the image.
     */
    [[deprecated]] PixelMap(ivec extent) noexcept : PixelMap(extent.x(), extent.y()) {}

    /** Construct an pixel-map.
    * This constructor will allocate its own memory.
    *
    * @param extent The width and height of the image.
    */
    PixelMap(ivec extent) noexcept : PixelMap(extent.x(), extent.y()) {}

    /** Construct an pixel-map from memory received from an API.
     * @param pixel A pointer to pixels received from the API.
     * @param width The width of the image.
     * @param height The height of the image.
     */
    PixelMap(T *pixels, int width, int height) noexcept : PixelMap(pixels, width, height, width) {}

    /** Construct an pixel-map from memory received from an API.
     * @param pixel A pointer to pixels received from the API.
     * @param extent The width and height of the image.
     */
    PixelMap(T *pixels, ivec extent) noexcept : PixelMap(pixels, extent.x(), extent.y()) {}

    /** Construct an pixel-map from memory received from an API.
     * @param pixel A pointer to pixels received from the API.
     * @param extent The width and height of the image.
     * @param stride Number of pixel elements until the next row.
     */
    PixelMap(T *pixels, ivec extent, int stride) noexcept : PixelMap(pixels, extent.x(), extent.y(), stride) {}

    gsl_suppress2(r.11,i.11)
    ~PixelMap() {
        if (selfAllocated) {
            delete[] pixels;
        }
    }

    /** Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
     */
    PixelMap(PixelMap const &other) = delete;

    PixelMap(PixelMap &&other) noexcept : pixels(other.pixels), width(other.width), height(other.height), stride(other.stride), selfAllocated(other.selfAllocated) {
        ttauri_assume(this != &other);
        other.selfAllocated = false;
    }

    operator bool() const noexcept {
        return pixels;
    }

    /** Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
     */
    PixelMap &operator=(PixelMap const &other) = delete;

    gsl_suppress2(r.11,i.11)
    PixelMap &operator=(PixelMap &&other) noexcept {
        ttauri_assume(this != &other);
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

    /** Get a (smaller) view of the map.
     * @param rect offset and extent of the rectangle to return.
     * @return A new pixel-map that point to the same memory as the current pixel-map.
     */
    PixelMap<T> submap(irect rect) const noexcept {
        ttauri_assert(
            (rect.x1() >= 0) &&
            (rect.y2() >= 0) &&
            (rect.width() >= 0) &&
            (rect.height() >= 0)
        );
        ttauri_assert(
            (rect.x2() <= width) &&
            (rect.y2() <= height)
        );

        let offset = rect.y1() * stride + rect.x1();

        if (rect.width() == 0 || rect.height() == 0) {
            // Image of zero width or height needs zero pixels returned.
            return { };
        } else {
            return { pixels + offset, rect.width(), rect.height(), stride };
        }
    }
    
    /** Get a (smaller) view of the map.
     * @param x x-offset in the current pixel-map
     * @param y y-offset in the current pixel-map
     * @param width width of the returned image.
     * @param height height of the returned image.
     * @return A new pixel-map that point to the same memory as the current pixel-map.
     */
    PixelMap<T> submap(int const x, int const y, int const width, int const height) const noexcept {
        return submap(irect{x, y, width, height});
    }

    PixelRow<T> const operator[](int const rowNr) const noexcept {
        return { pixels + (rowNr * stride), width };
    }

    PixelRow<T> operator[](int const rowNr) noexcept {
        return { pixels + (rowNr * stride), width };
    }

    PixelRow<T> const at(int const rowNr) const noexcept {
        ttauri_assert(rowNr < height);
        return (*this)[rowNr];
    }

    PixelRow<T> at(int const rowNr) noexcept {
        ttauri_assert(rowNr < height);
        return (*this)[rowNr];
    }

    /** Return a vector of pointers to rows.
     * The PNG API requires an array of pointers to write a png image to the pixel-map.
     */
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
