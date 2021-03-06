// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "geometry/axis_aligned_rectangle.hpp"
#include "geometry/extent.hpp"
#include <span>
#include <string>
#include <algorithm>
#include <vector>

namespace tt {

/** A row of pixels.
 */
template<typename T>
class pixel_row {
public:
    pixel_row(T *pixels, ssize_t width) noexcept : _pixels(pixels), _width(width) {}

    [[nodiscard]] ssize_t width() const noexcept
    {
        return _width;
    }

    /** Get a pointer to the pixel data.
     */
    [[nodiscard]] T const *data() const noexcept
    {
        return _pixels;
    }

    /** Get a pointer to the pixel data.
     */
    [[nodiscard]] T *data() noexcept
    {
        return _pixels;
    }

    /** Get a access to a pixel in the row.
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    [[nodiscard]] T const &operator[](ssize_t columnNr) const noexcept
    {
        return _pixels[columnNr];
    }

    /** Get a access to a pixel in the row.
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    [[nodiscard]] T &operator[](ssize_t columnNr) noexcept
    {
        return _pixels[columnNr];
    }

    /** Get a access to a pixel in the row.
     * This function does bound checking.
     *
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    [[nodiscard]] T const &at(ssize_t columnNr) const noexcept
    {
        tt_assert(columnNr >= 0 && columnNr < _width);
        return _pixels[columnNr];
    }

    /** Get a access to a pixel in the row.
     * This function does bound checking.
     *
     * @param columnNr The column number in the row.
     * @return a reference to a pixel.
     */
    [[nodiscard]] T &at(ssize_t columnNr) noexcept
    {
        tt_assert(columnNr >= 0 && columnNr < _width);
        return _pixels[columnNr];
    }

private:
    /** Pointer to an array of pixels.
     */
    T *_pixels;

    /** Number of pixels in the row.
     */
    ssize_t _width;
};

/** A 2D canvas of pixels.
 * This class may either allocate its own memory, or gives access
 * to memory allocated by another API, such as a Vulkan texture.
 */
template<typename T>
class pixel_map {
public:
    /** Construct an empty pixel-map.
     */
    pixel_map() noexcept : _pixels(nullptr), _width(0), _height(0), _stride(0), _self_allocated(true) {}

    /** Construct an pixel-map from memory received from an API.
     * @param pixels A pointer to pixels received from the API.
     * @param width The width of the image.
     * @param height The height of the image.
     * @param stride Number of pixel elements until the next row.
     */
    pixel_map(T *pixels, ssize_t width, ssize_t height, ssize_t stride) noexcept :
        _pixels(pixels), _width(width), _height(height), _stride(stride), _self_allocated(false)
    {
        tt_assert(_stride >= _width);
        tt_assert(_width >= 0);
        tt_assert(_height >= 0);

        if (pixels == nullptr) {
            _self_allocated = true;
            _pixels = new T[_height * _stride];
        }
    }

    /** Construct an pixel-map from memory received from an API.
     * @param pixels A pointer to pixels received from the API.
     * @param width The width of the image.
     * @param height The height of the image.
     * @param stride Number of pixel elements until the next row.
     */
    pixel_map(T *pixels, ssize_t width, ssize_t height) noexcept : pixel_map(pixels, width, height, width) {}

    /** Construct an pixel-map from memory received from an API.
     * @param pixels A pointer to pixels received from the API.
     * @param width The width of the image.
     * @param height The height of the image.
     */
    pixel_map(ssize_t width, ssize_t height, ssize_t stride) noexcept : pixel_map(nullptr, width, height, stride) {}

    /** Construct an pixel-map from memory received from an API.
     * @param pixels A pointer to pixels received from the API.
     * @param width The width of the image.
     * @param height The height of the image.
     */
    pixel_map(ssize_t width, ssize_t height) noexcept : pixel_map(nullptr, width, height, width) {}

    ~pixel_map()
    {
        if (_self_allocated) {
            delete[] _pixels;
        }
    }

    /** Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
     */
    pixel_map(pixel_map const &other) = delete;

    pixel_map copy() const noexcept
    {
        if (_self_allocated) {
            auto r = pixel_map(_width, _height);

            for (ssize_t y = 0; y != _height; ++y) {
                ttlet src_row = (*this)[y];
                auto dst_row = r[y];
                for (ssize_t x = 0; x != _width; ++x) {
                    dst_row[x] = src_row[x];
                }
            }

            return r;
        } else {
            return submap(0, 0, _width, _height);
        }
    }

    pixel_map(pixel_map &&other) noexcept :
        _pixels(other._pixels),
        _width(other._width),
        _height(other._height),
        _stride(other._stride),
        _self_allocated(other._self_allocated)
    {
        tt_axiom(this != &other);
        other._self_allocated = false;
    }

    [[nodiscard]] operator bool() const noexcept
    {
        return _pixels != nullptr;
    }

    [[nodiscard]] ssize_t width() const noexcept
    {
        return _width;
    }

    [[nodiscard]] ssize_t height() const noexcept
    {
        return _height;
    }

    [[nodiscard]] ssize_t stride() const noexcept
    {
        return _stride;
    }

    /** Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
     */
    pixel_map &operator=(pixel_map const &other) = delete;

    pixel_map &operator=(pixel_map &&other) noexcept
    {
        // Compatible with self-move.
        if (_self_allocated) {
            delete[] _pixels;
        }
        _pixels = other._pixels;
        _width = other._width;
        _height = other._height;
        _stride = other._stride;
        _self_allocated = other._self_allocated;
        other._self_allocated = false;
        return *this;
    }

    extent2 extent() const noexcept
    {
        return {narrow_cast<float>(_width), narrow_cast<float>(_height)};
    }

    /** Get a (smaller) view of the map.
     * @param x x-offset in the current pixel-map
     * @param y y-offset in the current pixel-map
     * @param width width of the returned image.
     * @param height height of the returned image.
     * @return A new pixel-map that point to the same memory as the current pixel-map.
     */
    pixel_map<T> submap(ssize_t x, ssize_t y, ssize_t width, ssize_t height) const noexcept
    {
        tt_axiom((x >= 0) && (y >= 0));
        tt_assert((x + width <= _width) && (y + height <= _height));

        ttlet offset = y * _stride + x;

        return {_pixels + offset, width, height, _stride};
    }

    pixel_map<T> submap(aarectangle rectangle) const noexcept
    {
        tt_axiom(round(rectangle) == rectangle);
        return submap(
            narrow_cast<ssize_t>(rectangle.left()),
            narrow_cast<ssize_t>(rectangle.bottom()),
            narrow_cast<ssize_t>(rectangle.width()),
            narrow_cast<ssize_t>(rectangle.height()));
    }

    pixel_row<T> const operator[](ssize_t rowNr) const noexcept
    {
        return {_pixels + (rowNr * _stride), _width};
    }

    pixel_row<T> operator[](ssize_t rowNr) noexcept
    {
        return {_pixels + (rowNr * _stride), _width};
    }

    pixel_row<T> const at(ssize_t rowNr) const noexcept
    {
        tt_assert(rowNr < _height);
        return (*this)[rowNr];
    }

    pixel_row<T> at(ssize_t rowNr) noexcept
    {
        tt_assert(rowNr < _height);
        return (*this)[rowNr];
    }

private:
    /** Pointer to a 2D canvas of pixels.
     */
    T *_pixels;

    /** Number of horizontal pixels.
     */
    ssize_t _width;

    /** Number of vertical pixels.
     */
    ssize_t _height;

    /** Number of pixel element until the next row.
     * This is used when the alignment of each row is different from the width of the canvas.
     */
    ssize_t _stride;

    /** True if the memory was allocated by this class, false if the canvas was received from another API.
     */
    bool _self_allocated;
};

template<typename T>
void copy(pixel_map<T> const &src, pixel_map<T> &dst) noexcept
{
    ssize_t width = std::min(src.width(), dst.width());
    ssize_t height = std::min(src.height(), dst.height());

    for (ssize_t y = 0; y != height; ++y) {
        ttlet src_row = src[y];
        auto dst_row = dst[y];
        for (ssize_t x = 0; x != width; ++x) {
            dst_row[x] = src_row[x];
        }
    }
}

template<int KERNEL_SIZE, typename KERNEL>
void horizontalFilterRow(pixel_row<uint8_t> row, KERNEL kernel) noexcept;

template<int KERNEL_SIZE, typename T, typename KERNEL>
void horizontalFilter(pixel_map<T> &pixels, KERNEL kernel) noexcept;

/*! Clear the pixels of this (sub)image.
 */
template<typename T>
void fill(pixel_map<T> &dst) noexcept;

/*! Fill with color.
 */
template<typename T>
void fill(pixel_map<T> &dst, T color) noexcept;

/*! Rotate an image 90 degrees counter-clockwise.
 */
template<typename T>
void rotate90(pixel_map<T> &dst, pixel_map<T> const &src) noexcept;

/*! Rotate an image 270 degrees counter-clockwise.
 */
template<typename T>
void rotate270(pixel_map<T> &dst, pixel_map<T> const &src) noexcept;

/*! Merge two image by applying std::max on each pixel.
 */
void mergeMaximum(pixel_map<uint8_t> &dst, pixel_map<uint8_t> const &src) noexcept;

/** Make a 1 pixel border on the edge of the pixel_map transparent
 * By copying the pixel value from just beyond the edge and setting
 * the alpha channel to zero. This allows bi-linear interpolation to
 * interpolate color correctly while anti-aliasing the edge.
 */
template<typename T>
inline void makeTransparentBorder(pixel_map<T> &pixel_map) noexcept;

} // namespace tt
