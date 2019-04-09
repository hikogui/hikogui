
#pragma once

#include "utils.hpp"

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

    constexpr PixelMap<T> submap(size_t const x, size_t const y, size_t const newWidth, size_t const newHeight) const {
        size_t const offset = y * stride + x;
        size_t const count = newHeight * stride + newWidth;

        if ((x + newWidth >= width) || (y + newHeight >= height)) {
            throw std::out_of_range("(x + newWidth >= width) || (y + newHeight >= height)");
        }
        return { pixels.subspan(offset, const), newWidth, newHeight, stride };
    }

    constexpr gsl::span<T> operator[](size_t const rowNr) const {
        return pixels.subspan(rownNr * stride, width);
    }

    constexpr gsl::span<T> at(const size_t rowNr) const {
        if (rowNr >= height) {
            throw std::out_of_range("rowNr >= height");
        }
        return *this[rowNr];
    }
};

struct Color_Alpha {
    uint8_t alpha;

    constexpr uint8_t a() const { return alpha; }
    constexpr float aLinear() const { return static_cast<float>(a()) / 255.0; }
};

struct Color_sRGBA {
    uint32_t color;

    constexpr uint8_t r() const { return (color >> 24) & 0xff; }
    constexpr uint8_t g() const { return (color >> 16) & 0xff; }
    constexpr uint8_t b() const { return (color >> 8) & 0xff; }
    constexpr uint8_t a() const { return color & 0xff; }

    constexpr float rLinear() const { return gammaToLinearTable[r()]; }
    constexpr float gLinear() const { return gammaToLinearTable[g()]; }
    constexpr float bLinear() const { return gammaToLinearTable[b()]; }
    constexpr float aLinear() const { return static_cast<float>(a()) / 255.0; }

    /*! Load the pixel into an SSE register.
     * The pixel will be in linear sRGBA clamped 0.0 - 1.0 format.
     */
    __m128 load_ps() const {
        return _mm_set_ps(
            gammaToLinearTable[r()],
            gammaToLinearTable[g()],
            gammaToLinearTable[b()],
            static_cast<float>(a()) / 255.0
        );
    }

    /* Store the pixel from an SSE register.
     * The resulting pixel is in sRGBA gamma corrected format.
     */
    void store_ps(__m128 x) {
        pack_sRGBA_ps(&color, sRGBA_gamma_ps(x));
    }
};

inline void compositeOver(Color_sRGBA &dst, const Color_sRGBA &below, const Color_sRGBA &above)
{
    auto const belowLinear = below.load_ps();
    auto const aboveLinear = above.load_ps();

    auto const r = compositeOver(belowLinear, aboveLinear);

    dst.store_ps(r);
}

/*! 
 */
inline void compositeOver(PixelMap<uint32_t> dstMap, const PixelMap<uint32_t> belowMap, const PixelMap<uint32_t> aboveMap)
{
  /*  auto const indices = _mm512_set_epi32(0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60);

    for (size_t rowNr = 0; rowNr < dstMap.height; rowNr++) {
        auto const dstRow = dstMap.at(rowNr);
        auto const belowRow = belowMap.at(rowNr);
        auto const aboveRow = aboveMap.at(rowNr);

        for (size_t columnNr = 0; columnNr < dstMap.width; columnNr += 8) {
            auto aboveRIndices = _mm512_i32gather_ps(indices, gammaToLinearTable.data(), sizeof (float));
            auto aboveR = _mm512_i32gather_ps(aboveRIndices, gammaToLinearTable.data(), sizeof (float));

                
            return 

            auto above = aboveRow[columnNr];
            auto below = belowRow[columnNr]; 
        
        }
    }*/
}

}