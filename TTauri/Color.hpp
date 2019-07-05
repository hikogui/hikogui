// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "geometry.hpp"
#include "utils.hpp"
#include <glm/glm.hpp>
#include <boost/format.hpp>
#include <boost/endian/conversion.hpp>
#include <string>
#include <algorithm>

namespace TTauri {

inline float linear_to_gamma_f32(float u)
{
    if (u <= 0.0031308f) {
        return u * 12.92f;
    } else {
        return std::pow(u, 1.0f/2.4f) * 1.055f - 0.055f;
    }
}

inline float gamma_to_linear_f32(float u)
{
    if (u <= 0.04045f) {
        return u / 12.92f;
    } else {
        return std::pow((u + 0.055f) / 1.055f, 2.4f);
    }
}

inline const auto gamma_to_linear_i16_table = generate_array<int16_t, 256>([](auto i) {
    let u = i / 255.0f;
    return static_cast<int16_t>(gamma_to_linear_f32(u) * 4095.0f + 0.5f);
});

inline int16_t gamma_to_linear_i16(uint8_t u)
{
    return gamma_to_linear_i16_table[u];
}

inline const auto linear_to_gamma_u8_table = generate_array<uint8_t, 4096>([](auto i) {
    let u = i / 4095.0f;
    return static_cast<uint8_t>(linear_to_gamma_f32(u) * 255.0f + 0.5f);
});

inline uint8_t linear_to_gamma_u8(int16_t u)
{
    if (u >= 4096) {
        return 255;
    } else if (u < 0) {
        return 0;
    } else {
        return linear_to_gamma_u8_table[u];
    }
}

inline uint8_t linear_alpha_u8(int16_t u)
{
    if (u < 0) {
        return 0;
    } else {
        return (static_cast<uint32_t>(u) * 255 + 128) / 32767;
    }
}

inline int16_t linear_alpha_i16(uint8_t u)
{
    return static_cast<int16_t>((static_cast<int32_t>(u) * 32767 + 128) / 255);
}

/*! Wide Gammut linear sRGB with pre-mulitplied alpha.
 * This RGB space is compatible with sRGB but can represent colours outside of the
 * sRGB gammut. Becuase it is linear and has pre-multiplied alpha it is easy to use
 * for compositing.
 */
struct wsRGBA {
    glm::i16vec4 color;

    static constexpr int64_t I64_MAX_ALPHA = 32767;
    static constexpr int64_t I64_MAX_COLOR = 32767;
    static constexpr int64_t I64_MAX_SRGB = 4095;
    static constexpr float F32_MAX_ALPHA = I64_MAX_ALPHA;
    static constexpr float F32_ALPHA_MUL = 1.0f / F32_MAX_ALPHA;
    static constexpr float F32_MAX_SRGB = I64_MAX_SRGB;
    static constexpr float F32_SRGB_MUL = 1.0f / F32_MAX_SRGB;

    wsRGBA() : color({0, 0, 0, 0}) {}

    /*! Set the colour using the pixel value.
     * No conversion is done with the given value.
     */
    wsRGBA(glm::i16vec4 c) :
        color(c) {}

    /*! Set the colour with linear-sRGB values.
     * sRGB values are between 0.0 and 1.0, values outside of the sRGB color gammut should be between -0.5 - 7.5.
     * This constructor expect colour which has not been pre-multiplied with the alpha.
     */
    wsRGBA(glm::vec4 c) :
        color(static_cast<glm::i16vec4>(glm::vec4{c.rgb * c.a * F32_MAX_SRGB, c.a * F32_MAX_ALPHA })) {}

    /*! Set the colour with linear-sRGB values.
     * sRGB values are between 0.0 and 1.0, values outside of the sRGB color gammut should be between -0.5 - 7.5.
     * This constructor expect colour which has not been pre-multiplied with the alpha.
     */
    wsRGBA(double r, double g, double b, double a=1.0) :
        wsRGBA(glm::vec4{r, g, b, a}) {}

    /*! Set the colour with gamma corrected sRGB values.
     */
    wsRGBA(uint32_t c) {
        let colorWithoutPreMultiply = glm::i64vec4{
            gamma_to_linear_i16((c >> 24) & 0xff),
            gamma_to_linear_i16((c >> 16) & 0xff),
            gamma_to_linear_i16((c >> 8) & 0xff),
            linear_alpha_i16(c & 0xff)
        };

        color = static_cast<glm::i16vec4>(
            glm::i64vec4(
                (colorWithoutPreMultiply.rgb * colorWithoutPreMultiply.a) / I64_MAX_ALPHA,
                colorWithoutPreMultiply.a
            )
        );
    }

    bool operator==(wsRGBA const &other) const {
        return color == other.color;
    }

    int16_t const &r() const { return color.r; }
    int16_t const &g() const { return color.g; }
    int16_t const &b() const { return color.b; }
    int16_t const &a() const { return color.a; }

    int16_t &r() { return color.r; }
    int16_t &g() { return color.g; }
    int16_t &b() { return color.b; }
    int16_t &a() { return color.a; }

    bool isTransparent() const { return color.a <= 0; }
    bool isOpaque() const { return color.a == I64_MAX_ALPHA; }

    /*! Return a linear wsRGBA float vector with pre multiplied alpha.
     */
    glm::vec4 to_wsRGBApm_vec4() const {
        let floatColor = static_cast<glm::vec4>(color);
        return { floatColor.rgb * F32_SRGB_MUL, floatColor.a * F32_ALPHA_MUL };
    }

    glm::vec4 to_Linear_sRGBA_vec4() const {
        let floatColor = to_wsRGBApm_vec4();

        if (floatColor.a == 0) {
            return { 0.0, 0.0, 0.0, 0.0 };
        } else {
            let oneOverAlpha = 1.0f / floatColor.a;
            return { floatColor.rgb * oneOverAlpha, floatColor.a };
        }
    }

    /*! Return a 32 bit gamma corrected sRGBA colour with normal alpha.
    */
    uint32_t to_sRGBA_u32() const {
        if (color.a == 0) {
            return 0;
        }

        let i64colorPM = static_cast<glm::i64vec4>(color);
        let i64color = glm::i64vec4{
            (i64colorPM.rgb * I64_MAX_ALPHA) / i64colorPM.a,
            i64colorPM.a
        };
        let i16color = static_cast<glm::i16vec4>(i64color);

        let red = linear_to_gamma_u8(i16color.r);
        let green = linear_to_gamma_u8(i16color.g);
        let blue = linear_to_gamma_u8(i16color.b);
        let alpha = linear_alpha_u8(i16color.a);
        return
            (static_cast<uint32_t>(red) << 24) |
            (static_cast<uint32_t>(green) << 16) |
            (static_cast<uint32_t>(blue) << 8) |
            static_cast<uint32_t>(alpha);
    }

    std::string string() const {
        let floatColor = to_wsRGBApm_vec4();
        if (
            floatColor.r >= 0.0 && floatColor.r <= 1.0 &&
            floatColor.g >= 0.0 && floatColor.g <= 1.0 &&
            floatColor.b >= 0.0 && floatColor.b <= 1.0
        ) {
            // This color is inside the sRGB gamut.
            return (boost::format("#%08x") % to_sRGBA_u32()).str();

        } else {
            return (boost::format("<%.3f, %.3f, %.3f, %.3f>") % floatColor.r % floatColor.g % floatColor.b % floatColor.a).str();
        }
    }

    void desaturate(int16_t brightness) {
        constexpr int64_t RY = static_cast<int64_t>(0.2126 * 32767.0);
        constexpr int64_t RG = static_cast<int64_t>(0.7152 * 32767.0);
        constexpr int64_t RB = static_cast<int64_t>(0.0722 * 32767.0);
        constexpr int64_t SCALE = static_cast<int64_t>(32767.0 * 2);

        int64_t y = ((RY * r() + RG * g() + RB * b()) * brightness) / SCALE;
        r() = g() = b() = static_cast<int16_t>(std::clamp(
            y,
            static_cast<int64_t>(std::numeric_limits<int16_t>::min()),
            static_cast<int64_t>(std::numeric_limits<int16_t>::max())
        ));
    }

    void composit(wsRGBA over) {
        if (over.isTransparent()) {
            return;
        }
        if (over.isOpaque()) {
            color = over.color;
            return;
        }

        // 15 bit
        constexpr int64_t OVERV_MAX = I64_MAX_COLOR;
        let overV = static_cast<glm::i64vec4>(over.color);

        // 15 bit
        constexpr int64_t UNDERV_MAX = I64_MAX_COLOR;
        let underV = static_cast<glm::i64vec4>(color);

        // 15 bit
        constexpr int64_t ONE = OVERV_MAX;
        constexpr int64_t ONEMINUSOVERALPHA_MAX = OVERV_MAX;
        let oneMinusOverAlpha = ONE - overV.a;

        static_assert(OVERV_MAX * ONE == UNDERV_MAX * ONEMINUSOVERALPHA_MAX);

        // 15 bit + 15 bit == 15 bit + 15 bit
        constexpr int64_t RESULTV_MAX = UNDERV_MAX * ONEMINUSOVERALPHA_MAX;
        let resultV = (overV * ONE) + (underV * oneMinusOverAlpha);

        constexpr int64_t RESULTV_DIVIDER = RESULTV_MAX / I64_MAX_COLOR;
        static_assert(RESULTV_DIVIDER == 0x7fff);
        color = static_cast<glm::i16vec4>(resultV / RESULTV_DIVIDER);
    }

    void composit(wsRGBA over, uint8_t mask) {
        constexpr int64_t MASK_MAX = 255;

        if (mask == 0) {
            return;
        } else if (mask == MASK_MAX) {
            return composit(over);
        } else {
            // calculate 'over' by multiplying all components with the new alpha.
            // This means that the color stays pre-multiplied.
            constexpr int64_t NEWOVERV_MAX = I64_MAX_COLOR * MASK_MAX;
            let newOverV = static_cast<glm::i64vec4>(over.color) * static_cast<int64_t>(mask);

            constexpr int64_t NEWOVERV_DIVIDER = NEWOVERV_MAX / I64_MAX_COLOR;
            let newOver = wsRGBA{ static_cast<glm::i16vec4>(newOverV / NEWOVERV_DIVIDER) };
            return composit(newOver);
        }
    }

    void subpixelComposit(wsRGBA over, glm::u8vec3 mask) {
        constexpr int64_t MASK_MAX = 255;

        if (mask.r == mask.g && mask.r == mask.b) {
            return composit(over, mask.r);
        }

        // 8 bit
        constexpr int64_t MASKV_MAX = MASK_MAX;
        let maskV = glm::i64vec4{
            mask,
            (static_cast<int64_t>(mask.r) + static_cast<int64_t>(mask.g) + static_cast<int64_t>(mask.b)) / 3
        };

        // 15 bit
        constexpr int64_t UNDERV_MAX = I64_MAX_COLOR;
        let underV = static_cast<glm::i64vec4>(color);

        // 15 bit
        constexpr int64_t _OVERV_MAX = I64_MAX_COLOR;
        let _overV = static_cast<glm::i64vec4>(over.color);

        // The over color was already pre-multiplied with it's own alpha, so
        // it only needs to be pre multiplied with the mask.
        // 15 bit + 8 bit = 23 bit
        constexpr int64_t OVER_MAX = _OVERV_MAX * MASKV_MAX;
        let overV = _overV * maskV;

        // The alpha for each component is the subpixel-mask multiplied by the alpha of the original over.
        // 8 bit + 15 bit = 23 bit
        constexpr int64_t ALPHAV_MAX = MASKV_MAX * _OVERV_MAX;
        let alphaV = maskV * _overV.a;

        // 23 bit
        constexpr int64_t ONEMINUSOVERALPHAV_MAX = ALPHAV_MAX;
        constexpr glm::i64vec4 oneV = { ALPHAV_MAX, ALPHAV_MAX, ALPHAV_MAX, ALPHAV_MAX };
        let oneMinusOverAlphaV = oneV - alphaV;


        // 23 bit + 15 bit == 15bit + 23bit == 38bit
        constexpr int64_t ONE = 0x7fff;
        static_assert(OVER_MAX * ONE == UNDERV_MAX * ONEMINUSOVERALPHAV_MAX);
        constexpr int64_t RESULTV_MAX = OVER_MAX * ONE;
        let resultV = (overV * ONE) + (underV * oneMinusOverAlphaV);

        // 38bit - 15bit = 23bit.
        constexpr int64_t RESULTV_DIVIDER = RESULTV_MAX / I64_MAX_COLOR;
        static_assert(RESULTV_DIVIDER == 0x7fff * 0xff);
        color = static_cast<glm::i16vec4>(resultV / RESULTV_DIVIDER);
    }
};

// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
const glm::mat3x3 matrix_sRGB_to_XYZ = {
    0.4124564, 0.3575761, 0.1804375,
    0.2126729, 0.7151522, 0.0721750,
    0.0193339, 0.1191920, 0.9503041
};

const glm::mat3x3 matrix_XYZ_to_sRGB = {
    3.2404542, -1.5371385, -0.4985314,
    -0.9692660, 1.8760108, 0.0415560,
    0.0556434, -0.2040259,  1.0572252
};


}

