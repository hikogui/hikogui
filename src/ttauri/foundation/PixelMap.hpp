// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/required.hpp"
#include "ttauri/foundation/iaarect.hpp"
#include <nonstd/span>
#include <string>
#include <algorithm>
#include <vector>

namespace tt {

/** A row of pixels.
 */
template <typename T>
struct PixelRow {
    /** Pointer to an array of pixels.
     */
    T *pixels;

    /** Number of pixels in the row.
     */
    ssize_t width;

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
    T const &operator[](ssize_t columnNr) const noexcept {
        return pixels[columnNr];
    }

    /** Get a access to a pixel in the row.
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    T &operator[](ssize_t columnNr) noexcept {
        return pixels[columnNr];
    }

    /** Get a access to a pixel in the row.
     * This function does bound checking.
     *
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    T const &at(ssize_t columnNr) const noexcept {
        tt_assert(columnNr >= 0 && columnNr < width);
        return pixels[columnNr];
    }

    /** Get a access to a pixel in the row.
     * This function does bound checking.
     *
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    T &at(ssize_t columnNr) noexcept {
        tt_assert(columnNr >= 0 && columnNr < width);
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
    ssize_t width;

    /** Number of vertical pixels.
     */
    ssize_t height;

    /** Number of pixel element until the next row.
     * This is used when the alignment of each row is different from the width of the canvas.
     */
    ssize_t stride;

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
    PixelMap(T *pixels, ssize_t width, ssize_t height, ssize_t stride) noexcept : pixels(pixels), width(width), height(height), stride(stride) {
        if (pixels) {
            tt_assert(stride >= width);
            tt_assert(width > 0);
            tt_assert(height > 0);
        } else {
            tt_assert(width == 0);
            tt_assert(height == 0);
        }
    } 

    /** Construct an pixel-map.
     * This constructor will allocate its own memory.
     *
     * @param width The width of the image.
     * @param height The height of the image.
     */
    gsl_suppress(r.11)
    PixelMap(ssize_t width, ssize_t height) noexcept : pixels(new T[width * height]), width(width), height(height), stride(width), selfAllocated(true) {
        if (pixels) {
            tt_assert(stride >= width);
            tt_assert(width > 0);
            tt_assert(height > 0);
        } else {
            tt_assert(width == 0);
            tt_assert(height == 0);
        }
    }

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
    PixelMap(T *pixels, ssize_t width, ssize_t height) noexcept : PixelMap(pixels, width, height, width) {}

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
    PixelMap(T *pixels, ivec extent, ssize_t stride) noexcept : PixelMap(pixels, extent.x(), extent.y(), stride) {}

    gsl_suppress2(r.11,i.11)
    ~PixelMap() {
        if (selfAllocated) {
            delete[] pixels;
        }
    }

    /** Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
     */
    PixelMap(PixelMap const &other) = delete;

    PixelMap copy() const noexcept {
        if (selfAllocated) {
            auto r = PixelMap(width, height);

            for (ssize_t y = 0; y != height; ++y) {
                ttlet src_row = (*this)[y];
                auto dst_row = r[y];
                for (ssize_t x = 0; x != width; ++x) {
                    dst_row[x] = src_row[x];
                }
            }

            return r;
        } else {
            return submap(0, 0, width, height);
        }
    }

    PixelMap(PixelMap &&other) noexcept : pixels(other.pixels), width(other.width), height(other.height), stride(other.stride), selfAllocated(other.selfAllocated) {
        tt_assume(this != &other);
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
        // Compatible with self-move.
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

    ivec extent() const noexcept {
        return {width, height};
    }

    /** Get a (smaller) view of the map.
     * @param rect offset and extent of the rectangle to return.
     * @return A new pixel-map that point to the same memory as the current pixel-map.
     */
    PixelMap<T> submap(iaarect rect) const noexcept {
        tt_assert(
            (rect.x1() >= 0) &&
            (rect.y2() >= 0) &&
            (rect.width() >= 0) &&
            (rect.height() >= 0)
        );
        tt_assert(
            (rect.x2() <= width) &&
            (rect.y2() <= height)
        );

        ttlet offset = rect.y1() * stride + rect.x1();

        return { pixels + offset, rect.width(), rect.height(), stride };
    }
    
    /** Get a (smaller) view of the map.
     * @param x x-offset in the current pixel-map
     * @param y y-offset in the current pixel-map
     * @param width width of the returned image.
     * @param height height of the returned image.
     * @return A new pixel-map that point to the same memory as the current pixel-map.
     */
    PixelMap<T> submap(ssize_t x, ssize_t y, ssize_t _width, ssize_t _height) const noexcept {
        return submap(iaarect{x, y, _width, _height});
    }

    PixelRow<T> const operator[](ssize_t rowNr) const noexcept {
        return { pixels + (rowNr * stride), width };
    }

    PixelRow<T> operator[](ssize_t rowNr) noexcept {
        return { pixels + (rowNr * stride), width };
    }

    PixelRow<T> const at(ssize_t rowNr) const noexcept {
        tt_assert(rowNr < height);
        return (*this)[rowNr];
    }

    PixelRow<T> at(ssize_t rowNr) noexcept {
        tt_assert(rowNr < height);
        return (*this)[rowNr];
    }
};

template<typename T>
void copy(PixelMap<T> const &src, PixelMap<T> &dst) noexcept {
    ssize_t width = std::min(src.width, dst.width);
    ssize_t height = std::min(src.height, dst.height);

    for (ssize_t y = 0; y != height; ++y) {
        ttlet src_row = src[y];
        auto dst_row = dst[y];
        for (ssize_t x = 0; x != width; ++x) {
            dst_row[x] = src_row[x];
        }
    }
}

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

/** Make a 1 pixel border on the edge of the pixelMap transparent
 * By copying the pixel value from just beyond the edge and setting
 * the alpha channel to zero. This allows bi-linear interpolation to
 * interpolate color correctly while anti-aliasing the edge.
 */
template<typename T>
inline void makeTransparentBorder(PixelMap<T> & pixelMap) noexcept;

}
