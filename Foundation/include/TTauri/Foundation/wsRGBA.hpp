// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/algorithm.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/vec.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <string>
#include <algorithm>
#include <array>

namespace TTauri {


extern const std::array<int16_t,256> gamma_to_linear_i16_table;

gsl_suppress2(bounds.2,bounds.4)
inline int16_t gamma_to_linear_i16(uint8_t u) noexcept
{
    return gamma_to_linear_i16_table[u];
}

extern const std::array<uint8_t,4096> linear_to_gamma_u8_table;

gsl_suppress2(bounds.2,bounds.4)
inline uint8_t linear_to_gamma_u8(int16_t u) noexcept
{
    if (u >= 4096) {
        return 255;
    } else if (u < 0) {
        return 0;
    } else {
        return linear_to_gamma_u8_table[u];
    }
}

inline uint8_t linear_alpha_u8(int16_t u) noexcept
{
    if (u < 0) {
        return 0;
    } else {
        return (static_cast<uint32_t>(u) * 255 + 128) / 32767;
    }
}

inline int16_t linear_alpha_i16(uint8_t u) noexcept
{
    return static_cast<int16_t>((int32_t{u} * 32767 + 128) / 255);
}

/*! Wide Gamut linear sRGB with pre-multiplied alpha.
 * This RGB space is compatible with sRGB but can represent colors outside of the
 * sRGB gamut. Because it is linear and has pre-multiplied alpha it is easy to use
 * for compositing.
 */
struct wsRGBA {
    std::array<int16_t,4> color;

    static constexpr int64_t I64_MAX_ALPHA = 32767;
    static constexpr int64_t I64_MAX_COLOR = 32767;
    static constexpr int64_t I64_MAX_SRGB = 4095;
    static constexpr float F32_MAX_ALPHA = I64_MAX_ALPHA;
    static constexpr float F32_ALPHA_MUL = 1.0f / F32_MAX_ALPHA;
    static constexpr float F32_MAX_SRGB = I64_MAX_SRGB;
    static constexpr float F32_SRGB_MUL = 1.0f / F32_MAX_SRGB;

    wsRGBA() noexcept : color({0, 0, 0, 0}) {}

    /*! Set the color using the pixel value.
     * No conversion is done with the given value.
     */
    explicit wsRGBA(std::array<int16_t,4> c) noexcept :
        color(c) {}

    wsRGBA(vec const &rhs) noexcept {
        let max_mmmm = _mm_set_ps1(F32_MAX_SRGB);
        let max_000m = _mm_set_ss(F32_MAX_ALPHA);
        let max_abgr = _mm_insert_ps(max_mmmm, max_000m, 0b00'11'0000);
        let color_fp = (rhs._1aaa() * rhs) * vec{max_abgr};
        let color_i32 = _mm_cvtps_epi32(color_fp);
        let color_i16 = _mm_packs_epi32(color_i32, color_i32);
        _mm_storeu_si64(color.data(), color_i16);
    }

    wsRGBA &operator=(vec const &rhs) noexcept {
        let max_mmmm = _mm_set_ps1(F32_MAX_SRGB);
        let max_000m = _mm_set_ss(F32_MAX_ALPHA);
        let max_abgr = _mm_insert_ps(max_mmmm, max_000m, 0b00'11'0000);
        let color_fp = (rhs._1aaa() * rhs) * vec{max_abgr};
        let color_i32 = _mm_cvtps_epi32(color_fp);
        let color_i16 = _mm_packs_epi32(color_i32, color_i32);
        _mm_storeu_si64(color.data(), color_i16);
        return *this;
    }

    /*! Set the color with linear-sRGB values.
     * sRGB values are between 0.0 and 1.0, values outside of the sRGB color gamut should be between -0.5 - 7.5.
     * This constructor expect color which has not been pre-multiplied with the alpha.
     */
    [[deprecated]] explicit wsRGBA(double r, double g, double b, double a=1.0) noexcept :
        wsRGBA(vec{r, g, b, a}) {}

    /** Return floating point values.
     * Alpha is not pre-multiplied.
     */
    explicit operator vec () const noexcept {
        let floatColor = vec(color[0], color[1], color[2], color[3]);
        if (floatColor.a() == 0.0) {
            return {0.0, 0.0, 0.0, 0.0};
        } else {
            let alpha = floatColor.a() * F32_ALPHA_MUL;
            let oneOverAlpha = 1.0f / alpha;
            return (floatColor * F32_SRGB_MUL * oneOverAlpha).a(alpha);
        }
    }

    int16_t const &r() const noexcept { return color[0]; }
    int16_t const &g() const noexcept { return color[1]; }
    int16_t const &b() const noexcept { return color[2]; }
    int16_t const &a() const noexcept { return color[3]; }

    int16_t &r() noexcept { return color[0]; }
    int16_t &g() noexcept { return color[1]; }
    int16_t &b() noexcept { return color[2]; }
    int16_t &a() noexcept { return color[3]; }

    int16_t operator[](size_t i) const {
        return color[i];
    }

    int16_t &operator[](size_t i) {
        return color[i];
    }

    bool isTransparent() const noexcept { return color[3] <= 0; }
    bool isOpaque() const noexcept { return color[3] == I64_MAX_ALPHA; }

    /*! Return a 32 bit gamma corrected sRGBA colour with normal alpha.
    */
    uint32_t to_sRGBA_u32() const noexcept {
        let i64colorPM = std::array<int64_t,4>{color[0], color[1], color[2], color[3]};
        if (i64colorPM[3] == 0) {
            return 0;
        }

        let i64color = std::array<int64_t,4>{
            (i64colorPM[0] * I64_MAX_ALPHA) / i64colorPM[3],
            (i64colorPM[1] * I64_MAX_ALPHA) / i64colorPM[3],
            (i64colorPM[2] * I64_MAX_ALPHA) / i64colorPM[3],
            i64colorPM[3]
        };
        let i16color = std::array<int16_t,4>{
            static_cast<int16_t>(i64color[0]),
            static_cast<int16_t>(i64color[1]),
            static_cast<int16_t>(i64color[2]),
            static_cast<int16_t>(i64color[3])
        };

        let red = linear_to_gamma_u8(i16color[0]);
        let green = linear_to_gamma_u8(i16color[1]);
        let blue = linear_to_gamma_u8(i16color[2]);
        let alpha = linear_alpha_u8(i16color[3]);
        return
            (static_cast<uint32_t>(red) << 24) |
            (static_cast<uint32_t>(green) << 16) |
            (static_cast<uint32_t>(blue) << 8) |
            static_cast<uint32_t>(alpha);
    }

    void desaturate(uint16_t brightness) noexcept {
        constexpr int64_t RY = static_cast<int64_t>(0.2126 * 32768.0);
        constexpr int64_t RG = static_cast<int64_t>(0.7152 * 32768.0);
        constexpr int64_t RB = static_cast<int64_t>(0.0722 * 32768.0);
        constexpr int64_t SCALE = static_cast<int64_t>(32768 * 32768);

        let _r = static_cast<int64_t>(r());
        let _g = static_cast<int64_t>(g());
        let _b = static_cast<int64_t>(b());

        int64_t y = ((
            RY * _r +
            RG * _g +
            RB * _b
        ) * brightness) / SCALE;
        r() = g() = b() = static_cast<int16_t>(std::clamp(
            y,
            static_cast<int64_t>(std::numeric_limits<int16_t>::min()),
            static_cast<int64_t>(std::numeric_limits<int16_t>::max())
        ));
    }

    void composit(wsRGBA over) noexcept {
        if (over.isTransparent()) {
            return;
        }
        if (over.isOpaque()) {
            color = over.color;
            return;
        }

        // 15 bit
        constexpr int64_t OVERV_MAX = I64_MAX_COLOR;
        let overV = std::array<int64_t,4>{over.color[0], over.color[1], over.color[2], over.color[3]};

        // 15 bit
        constexpr int64_t UNDERV_MAX = I64_MAX_COLOR;
        let underV = std::array<int64_t,4>{color[0], color[1], color[2], color[3]};

        // 15 bit
        constexpr int64_t ONE = OVERV_MAX;
        constexpr int64_t ONEMINUSOVERALPHA_MAX = OVERV_MAX;
        let oneMinusOverAlpha = ONE - overV[3];

        static_assert(OVERV_MAX * ONE == UNDERV_MAX * ONEMINUSOVERALPHA_MAX);

        // 15 bit + 15 bit == 15 bit + 15 bit
        constexpr int64_t RESULTV_MAX = UNDERV_MAX * ONEMINUSOVERALPHA_MAX;
        let resultV = std::array<int64_t,4>{
            (overV[0] * ONE) + (underV[0] * oneMinusOverAlpha),
            (overV[1] * ONE) + (underV[1] * oneMinusOverAlpha),
            (overV[2] * ONE) + (underV[2] * oneMinusOverAlpha),
            (overV[3] * ONE) + (underV[3] * oneMinusOverAlpha)
        };

        constexpr int64_t RESULTV_DIVIDER = RESULTV_MAX / I64_MAX_COLOR;
        static_assert(RESULTV_DIVIDER == 0x7fff);
        color = std::array<int16_t,4>{
            static_cast<int16_t>(resultV[0] / RESULTV_DIVIDER),
            static_cast<int16_t>(resultV[1] / RESULTV_DIVIDER),
            static_cast<int16_t>(resultV[2] / RESULTV_DIVIDER),
            static_cast<int16_t>(resultV[3] / RESULTV_DIVIDER)
        };
    }

    void composit(wsRGBA over, uint8_t mask) noexcept {
        constexpr int64_t MASK_MAX = 255;

        if (mask == 0) {
            return;
        } else if (mask == MASK_MAX) {
            return composit(over);
        } else {
            // calculate 'over' by multiplying all components with the new alpha.
            // This means that the color stays pre-multiplied.
            constexpr int64_t NEWOVERV_MAX = I64_MAX_COLOR * MASK_MAX;
            let newOverV = std::array<int64_t,4>{
                static_cast<int64_t>(over.color[0]) * static_cast<int64_t>(mask),
                static_cast<int64_t>(over.color[1]) * static_cast<int64_t>(mask),
                static_cast<int64_t>(over.color[2]) * static_cast<int64_t>(mask),
                static_cast<int64_t>(over.color[3]) * static_cast<int64_t>(mask)
            };

            constexpr int64_t NEWOVERV_DIVIDER = NEWOVERV_MAX / I64_MAX_COLOR;
            let newOver = wsRGBA{ std::array<int16_t,4>{
                static_cast<int16_t>(newOverV[0] / NEWOVERV_DIVIDER),
                static_cast<int16_t>(newOverV[1] / NEWOVERV_DIVIDER),
                static_cast<int16_t>(newOverV[2] / NEWOVERV_DIVIDER),
                static_cast<int16_t>(newOverV[3] / NEWOVERV_DIVIDER)
            }};
            return composit(newOver);
        }
    }

    void subpixelComposit(wsRGBA over, std::array<uint8_t,3> mask) noexcept {
        constexpr int64_t MASK_MAX = 255;

        if (mask[0] == mask[1] && mask[0] == mask[2]) {
            return composit(over, mask[0]);
        }

        // 8 bit
        constexpr int64_t MASKV_MAX = MASK_MAX;
        let maskV = std::array<int64_t,4>{
            mask[0],
            mask[1],
            mask[2],
            (static_cast<int64_t>(mask[0]) + static_cast<int64_t>(mask[1]) + static_cast<int64_t>(mask[2])) / 3
        };

        // 15 bit
        constexpr int64_t UNDERV_MAX = I64_MAX_COLOR;
        let underV = std::array<int64_t,4>{color[0], color[1], color[2], color[3]};

        // 15 bit
        constexpr int64_t _OVERV_MAX = I64_MAX_COLOR;
        let _overV = std::array<int64_t,4>{over.color[0], over.color[1], over.color[2], over.color[3]};

        // The over color was already pre-multiplied with it's own alpha, so
        // it only needs to be pre multiplied with the mask.
        // 15 bit + 8 bit = 23 bit
        constexpr int64_t OVER_MAX = _OVERV_MAX * MASKV_MAX;
        let overV = std::array<int64_t,4>{
            _overV[0] * maskV[0],
            _overV[1] * maskV[1],
            _overV[2] * maskV[2],
            _overV[3] * maskV[3]
        };

        // The alpha for each component is the subpixel-mask multiplied by the alpha of the original over.
        // 8 bit + 15 bit = 23 bit
        constexpr int64_t ALPHAV_MAX = MASKV_MAX * _OVERV_MAX;
        let alphaV = std::array<int64_t,4>{
            maskV[0] * _overV[3],
            maskV[1] * _overV[3],
            maskV[2] * _overV[3],
            maskV[3] * _overV[3]
        };

        // 23 bit
        constexpr int64_t ONEMINUSOVERALPHAV_MAX = ALPHAV_MAX;
        let oneMinusOverAlphaV = std::array<int64_t,4>{
            ALPHAV_MAX - alphaV[0],
            ALPHAV_MAX - alphaV[1],
            ALPHAV_MAX - alphaV[2],
            ALPHAV_MAX - alphaV[3]
        };


        // 23 bit + 15 bit == 15bit + 23bit == 38bit
        constexpr int64_t ONE = 0x7fff;
        static_assert(OVER_MAX * ONE == UNDERV_MAX * ONEMINUSOVERALPHAV_MAX);
        constexpr int64_t RESULTV_MAX = OVER_MAX * ONE;
        let resultV = std::array<int64_t,4>{
            (overV[0] * ONE) + (underV[0] * oneMinusOverAlphaV[0]),
            (overV[1] * ONE) + (underV[1] * oneMinusOverAlphaV[1]),
            (overV[2] * ONE) + (underV[2] * oneMinusOverAlphaV[2]),
            (overV[3] * ONE) + (underV[3] * oneMinusOverAlphaV[3])
        };

        // 38bit - 15bit = 23bit.
        constexpr int64_t RESULTV_DIVIDER = RESULTV_MAX / I64_MAX_COLOR;
        static_assert(RESULTV_DIVIDER == 0x7fff * 0xff);
        color = std::array<int16_t,4>{
            static_cast<int16_t>(resultV[0] / RESULTV_DIVIDER),
            static_cast<int16_t>(resultV[1] / RESULTV_DIVIDER),
            static_cast<int16_t>(resultV[2] / RESULTV_DIVIDER),
            static_cast<int16_t>(resultV[3] / RESULTV_DIVIDER)
        };
    }

    friend bool operator==(wsRGBA const &lhs, wsRGBA const &rhs) noexcept
    {
        return lhs.color == rhs.color;
    }

    friend bool operator<(wsRGBA const &lhs, wsRGBA const &rhs) noexcept
    {
        if (lhs.color[0] != rhs.color[0]) {
            return lhs.color[0] < rhs.color[0];
        } else if (lhs.color[1] != rhs.color[1]) {
            return lhs.color[1] < rhs.color[1];
        } else if (lhs.color[2] != rhs.color[2]) {
            return lhs.color[2] < rhs.color[2];
        } else if (lhs.color[3] != rhs.color[3]) {
            return lhs.color[3] < rhs.color[3];
        } else {
            return false;
        }
    }

    friend bool operator!=(wsRGBA const &lhs, wsRGBA const &rhs) noexcept { return !(lhs == rhs); }
    friend bool operator>(wsRGBA const &lhs, wsRGBA const &rhs) noexcept { return rhs < lhs; }
    friend bool operator<=(wsRGBA const &lhs, wsRGBA const &rhs) noexcept { return !(lhs > rhs); }
    friend bool operator>=(wsRGBA const &lhs, wsRGBA const &rhs) noexcept { return !(lhs < rhs); }

    friend std::string to_string(wsRGBA const &x) noexcept
    {
        let floatColor = static_cast<vec>(x);
        if (
            floatColor.r() >= 0.0 && floatColor.r() <= 1.0 &&
            floatColor.g() >= 0.0 && floatColor.g() <= 1.0 &&
            floatColor.b() >= 0.0 && floatColor.b() <= 1.0
            ) {
            // This color is inside the sRGB gamut.
            return fmt::format("#{:08x}", x.to_sRGBA_u32());

        } else {
            return fmt::format("rgba{}", floatColor);
        }
    }
};


}

namespace std {

template<>
class hash<TTauri::wsRGBA> {
public:
    size_t operator()(TTauri::wsRGBA const &v) const {
        return
            hash<int16_t>{}(v[0]) ^
            hash<int16_t>{}(v[1]) ^
            hash<int16_t>{}(v[2]) ^
            hash<int16_t>{}(v[3]);
    }
};

}

