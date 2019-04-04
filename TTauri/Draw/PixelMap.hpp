
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

    constexpr PixelMap(gsl::span<T> pixels, size_t width, size_t height) :
        pixels(std::move(pixels)), width(width), height(height)
    {
        if (width * height >= pixels.size()) {
            throw std::out_of_range("Width * height >= number of pixels");
        }
    }

    constexpr gsl::span<T> operator[](const size_t rowNr) const {
        return pixels.subspan(rownNr * width, width);
    }

    constexpr gsl::span<T> at(const size_t rowNr) const {
        if (rowNr > height) {
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
inline void compositeOver(PixelMap<Color_sRGBA> dst, const PixelMap<Color_sRGBA> srcBelow, const PixelMap<Color_sRGBA> srcAbove)
{
    BOOST_ASSERT(dst.width == srcBelow.width);
    BOOST_ASSERT(dst.width == srcAbove.width);
    BOOST_ASSERT(dst.height == srcBelow.height);
    BOOST_ASSERT(dst.height == srcAbove.height);

    auto const nrPixels = dst.width * dst.height;
    for (size_t i = 0; i < nrPixels; i++) {
        auto &dstPixel = dst.pixels[i];
        auto const srcPixelBelow = srcBelow.pixels[i];
        auto const srcPixelAbove = srcAbove.pixels[i];
        compositeOver(dstPixel, srcPixelBelow, srcPixelAbove);
    }
}

}