
#pragma once

//#include "utils.hpp"

#include "TTauri/utils.hpp"
#include "TTauri/geometry.hpp"

#include <glm/glm.hpp>
#include <gsl/gsl>
#include <string>

#include <immintrin.h>


namespace TTauri::Draw {

template <typename T>
struct PixelMap {
    gsl::span<T> pixels;
    size_t width;
    size_t height;

    //! Stride in number of pixels; width of the original image.
    size_t stride;

    PixelMap() : pixels({}), width(0), height(0), stride(0) {}
    PixelMap(gsl::span<T> pixels, size_t width, size_t height) : pixels(pixels), width(width), height(height), stride(width) {}
    PixelMap(gsl::span<T> pixels, size_t width, size_t height, size_t stride) : pixels(pixels), width(width), height(height), stride(stride) {}

    constexpr PixelMap<T> submap(size_t const x, size_t const y, size_t const newWidth, size_t const newHeight) const {
        size_t const offset = y * stride + x;
        size_t const count = newHeight * stride + newWidth;

        if ((x + newWidth >= width) || (y + newHeight >= height)) {
            throw std::out_of_range("(x + newWidth >= width) || (y + newHeight >= height)");
        }
        return { pixels.subspan(offset, count), newWidth, newHeight, stride };
    }

    constexpr gsl::span<T> operator[](size_t const rowNr) const {
        return pixels.subspan(rowNr * stride, width);
    }

    constexpr gsl::span<T> at(const size_t rowNr) const {
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
};


}