
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
    PixelMap(gsl::span<T> pixels, glm::u64vec2 extent) : pixels(pixels), width(extent.x), height(extent.y), stride(extent.x) {}
    PixelMap(gsl::span<T> pixels, glm::u64vec2 extent, size_t stride) : pixels(pixels), width(extent.x), height(extent.y), stride(stride) {}

    constexpr PixelMap<T> submap(u64rect rect) const {
        size_t const offset = rect.offset.y * stride + rect.offset.x;
        size_t const count = (rect.extent.y - 1) * stride + rect.extent.x;

        if ((rect.offset.x + rect.extent.x > width) || (rect.offset.y + rect.extent.y > height)) {
            throw std::out_of_range("(rect.offset.x + rect.extent.x > width) || (rect.offset.y + rect.extent.y > height)");
        }
        return { pixels.subspan(offset, count), rect.extent.x, rect.extent.y, stride };
    }
    
    constexpr PixelMap<T> submap(size_t const x, size_t const y, size_t const width, size_t const height) const {
        return submap({{x, y}, {width, height}});
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

inline void add1PixelTransparentBorder(PixelMap<uint32_t> pixelMap)
{

    uint8_t const u8invisibleMask[4] = {0xff, 0xff, 0xff, 0};
    uint32_t u32invisibleMask;
    std::memcpy(&u32invisibleMask, u8invisibleMask, sizeof(u32invisibleMask));

    auto const topBorder = pixelMap.at(0);
    auto const topRow = pixelMap.at(1);
    auto const bottomRow = pixelMap.at(pixelMap.height - 2);
    auto const bottomBorder = pixelMap.at(pixelMap.height - 1);
    for (size_t x = 1; x < pixelMap.width - 1; x++) {
        topBorder[x] = topRow[x] & u32invisibleMask;
        bottomBorder[x] = bottomRow[x] & u32invisibleMask;
    }

    auto const rightBorderY = pixelMap.width - 1;
    auto const rightY = pixelMap.width - 2;
    for (size_t y = 1; y < pixelMap.height - 1; y++) {
        auto const row = pixelMap[y];
        row[0] = row[1] & u32invisibleMask;
        row[rightBorderY] = row[rightY] & u32invisibleMask;
    }

    pixelMap[0][0] = pixelMap[1][1] & u32invisibleMask;
    pixelMap[0][pixelMap.width - 1] = pixelMap[1][pixelMap.width - 2] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][0] = pixelMap[pixelMap.height - 2][1] & u32invisibleMask;
    pixelMap[pixelMap.height - 1][pixelMap.width - 1] = pixelMap[pixelMap.height - 2][pixelMap.width - 2] & u32invisibleMask;
}

}