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
struct wsRGBApm;
}

namespace TTauri::Draw {

template <typename T>
struct PixelRow {
    gsl::span<T> const pixels;
    size_t const width;

    T const* data() const {
        return pixels.data();
    }

    T * data() {
        return pixels.data();
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
        return pixels.at(columnNr);
    }

    T &at(size_t columnNr) {
        if (columnNr >= width) {
            throw std::out_of_range("columnNr >= height");
        }
        return pixels.at(columnNr);
    }
};

template <typename T>
struct PixelMap {
    bool selfAllocated = false;
    gsl::span<T> pixels;
    size_t width;
    size_t height;

    //! Stride in number of pixels; width of the original image.
    size_t stride;

    PixelMap() :
        pixels({}),
        width(0),
        height(0),
        stride(0) {}

    PixelMap(gsl::span<T> pixels, size_t width, size_t height, size_t stride) :
        pixels(pixels),
        width(width),
        height(height),
        stride(stride) {}

    PixelMap(size_t width, size_t height) :
        pixels({ new T[width * height], boost::numeric_cast<ptrdiff_t>(width * height) }),
        width(width),
        height(height),
        stride(width),
        selfAllocated(true) {}

    PixelMap(u64extent2 extent) : PixelMap(extent.width(), extent.height()) {}
    PixelMap(gsl::span<T> pixels, size_t width, size_t height) : PixelMap(pixels, width, height, width) {}
    PixelMap(gsl::span<T> pixels, u64extent2 extent) : PixelMap(pixels, extent.width(), extent.height()) {}
    PixelMap(gsl::span<T> pixels, u64extent2 extent, size_t stride) : PixelMap(pixels, extent.width(), extent.height(), stride) {}

    ~PixelMap() {
        if (selfAllocated) {
            delete[] pixels.data();
        }
    }

    /*! Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
     */
    PixelMap(PixelMap const &other) = delete;
    PixelMap(PixelMap &&other) : pixels(other.pixels), width(other.width), height(other.height), stride(other.stride), selfAllocated(other.selfAllocated) {
        other.selfAllocated = false;
    }

    /*! Disallowing copying so that life-time of selfAllocated pixels is easy to understand.
    */
    PixelMap &operator=(PixelMap const &other) = delete;
    PixelMap &operator=(PixelMap &&other) {
        if (selfAllocated) {
            delete[] pixels.data();
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

    PixelRow<T> const operator[](size_t const rowNr) const {
        return { pixels.subspan(rowNr * stride, width), width };
    }

    PixelRow<T> operator[](size_t const rowNr) {
        return { pixels.subspan(rowNr * stride, width), width };
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

    /*! Clear the pixels of this (sub)image.
     */
    void clear() {
        for (size_t rowNr = 0; rowNr < height; rowNr++) {
            auto row = this->at(rowNr);
            void *data = row.data();
            memset(data, 0, row.width * sizeof(T));
        }
    }

    /*! Fill with color.
     */
    void fill(T color) {
        for (size_t rowNr = 0; rowNr < height; rowNr++) {
            auto row = this->at(rowNr);
            for (size_t columnNr = 0; columnNr < width; columnNr++) {
                row[columnNr] = color;
            }
        }
    }
};



}
