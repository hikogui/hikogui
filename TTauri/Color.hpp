// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <glm/glm.hpp>
#include <boost/format.hpp>
#include <boost/endian/conversion.hpp>
#include <string>

namespace TTauri {

/*! Wide Gammut linear sRGB with pre-mulitplied alpha.
 * This RGB space is compatible with sRGB but can represent colours outside of the
 * sRGB gammut. Becuase it is linear and has pre-multiplied alpha it is easy to use
 * for compositing.
 */
struct wsRGBApm {
    glm::i16vec4 color;

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
        color(static_cast<i16vec4>(glm::vec4(c.rgb * c.a) * 4095.0f)) {}

    /*! Set the colour with linear-sRGB values.
     * sRGB values are between 0.0 and 1.0, values outside of the sRGB color gammut should be between -0.5 - 7.5.
     * This constructor expect colour which has not been pre-multiplied with the alpha.
     */
    wsRGBA(double r, double g, double b, double a) : wsRGBA({r, g, b, a}) {}

    int16_t const &r() const { return color.r; }
    int16_t const &g() const { return color.g; }
    int16_t const &b() const { return color.b; }
    int16_t const &a() const { return color.a; }

    int16_t &r() { return color.r; }
    int16_t &g() { return color.g; }
    int16_t &b() { return color.b; }
    int16_t &a() { return color.a; }

    bool isTransparant() const { return color.a <= 0; }
    bool isOpaque() const { return color.a >= 4095; }

    std::string string() const {
        let floatColor = to_i64vec4();
        return (boost::format("<%.3f, %.3f, %.3f, %.3f>") % floatColor.r % floatColor.g % floatColor.b % floatColor.a).str();
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
        constexpr int64_t overVmax = 0x7fff;
        let overV = static_cast<glm::i64vec4>(over.color);

        // 15 bit
        constexpr int64_t underVmax = 0x7fff;
        let underV = static_cast<glm::i64vec4>(color);

        // 15 bit
        constexpr int64_t oneMinusOverAlphaMax = 0x7fff;
        let oneMinusOverAlpha = overVmax - overV.a();

        // 15 bit + 15 bit == 15 bit + 15 bit
        static_assert(overVmax * 0x7fff == underVmax * oneMinusOverAlphaMax);
        constexpr int64_t resultVmax = underVmax * oneMinusOverAlphaMax;
        let resultV = overV * 0x7fff + underV * oneMinusOverAlphaMax;

        constexpr int64_t resultVdivider = resultVmax / 0x7fff;
        color = static_cast<glm::i16vec4>(resultV / resultVdivider);
    }

    void subpixelComposit(wsRGBA over, glm::u8vec3 mask) {
        if (mask.r == mask.g && mask.r == mask.b) {
            if (mask.r == 0) {
                return;
            } else if (mask.r == 255) {
                return composit(over);
            } else {
                // calculate 'over' by multiplying all components with the new alpha.
                // This means that the color stays pre-multiplied.
                constexpr int64_t newOverVmax = 0x7fff * 0xff;
                let newOverV = static_cast<glm::i64vec4>(over.color) * mask.r;

                constexpr int64_t newOverVdivider = newOverVmax / 0x7fff;
                let newOver = { static_cast<glm::i16vec4>(newOverV / newOverVdivider) };
                return composit(newOver);
            }
        }

        // 8 bit
        constexpr int64_t maskVmax = 0xff;
        let maskV = glm::i64vec4{
            mask,
            (static_cast<int64_t>(mask.r) + static_cast<int64_t>(mask.g) + static_cast<int64_t>(mask.b)) / 3
        };

        // 15 bit
        constexpr int64_t underVmax = 0x7fff;
        let underV = static_cast<glm::i64vec4>(color);

        // 15 bit
        constexpr int64_t _overVmax = 0x7fff;
        let _overV = static_cast<glm::i64vec4>(over.color);

        // The over color was already pre-multiplied with it's own alpha, so
        // it only needs to be pre multiplied with the mask.
        // 15 bit * 23 bit = 38 bit
        constexpr int64_t overVmax = _overVmax * maskVmax;
        let overV = _overV * maskV;

        // The alpha for each component is the subpixel-mask multiplied by the alpha of the original over.
        // 8 bit * 15 bit = 23 bit
        constexpr int64_t alphaVmax = maskVmax * _overVmax;
        let alphaV = maskV * _overV.a;

        // 23 bit
        constexpr int64_t oneMinusOverAlphaVmax = alphaVmax;
        let oneMinusOverAlphaV = glm::i64vec4{ alphaVmax, alphaVmax, alphaVmax, alphaVmax } - alphaV;

        // 38 bit == 15bit + 23bit
        static_assert(overVmax == underVmax * oneMinusOverAlphaV);
        constexpr int64_t resultVmax = overVmax;
        let resultV = overV + underV * oneMinusMaskV;

        constexpr int64_t resultVdivider = resultVmax / 0x7fff;
        color = static_cast<glm::i16vec4>(resultV / resultVdivider);
    }
};

struct sRGBA {
    glm::u8vec4 color;

    sRGBA() : color({0, 0, 0, 0}) {}

    sRGBA(uint32_t x) : color(
};


enum class ColorSpace {
    sRGB,
    XYZ
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

template<ColorSpace COLORSPACE, bool LINEAR>
struct Color {
    static const ColorSpace colorSpace = COLORSPACE;
    static const bool linear = LINEAR;

    glm::vec4 value;

    Color() : value({0.0, 0.0, 0.0, 0.0}) {}

    Color(glm::vec4 value) : value(value) {}

    Color(const Color &color) : value(color.value) {} 

    /*! Convert a uint32_t value to a colour.
     * The uint32_t value is split into 4 bytes. MSB->LSB: Red, Green, Blue, Alpha component.
     */
    Color(uint32_t rgba) :
        value({
            static_cast<float>((rgba >> 24) & 0xff) / 255.0,
            static_cast<float>((rgba >> 16) & 0xff) / 255.0,
            static_cast<float>((rgba >> 8) & 0xff) / 255.0,
            static_cast<float>(rgba & 0xff) / 255.0
        }) {}

    Color const &operator=(const Color &color) {
        value = color.value;
        return *this;
    }

    float r() const { return value.r; }
    float g() const { return value.g; }
    float b() const { return value.b; }
    float a() const { return value.a; }
    glm::vec3 rgb() const { return value.rgb; }

    std::string str() const {
        uint32_t tmp = toUInt32();
        return (boost::format("#%08x") % tmp).str();
    }

    static Color readPixel(uint32_t const &v) {
        return Color(boost::endian::big_to_native(v));
    }

    uint32_t writePixel() const {
        return boost::endian::native_to_big(toUInt32());
    }

    uint32_t toUInt32() const {
        return (
            (static_cast<uint32_t>(std::clamp(value.r, 0.0f, 1.0f) * 255.0f) << 24) |
            (static_cast<uint32_t>(std::clamp(value.g, 0.0f, 1.0f) * 255.0f) << 16) |
            (static_cast<uint32_t>(std::clamp(value.b, 0.0f, 1.0f) * 255.0f) << 8) |
            static_cast<uint32_t>(std::clamp(value.a, 0.0f, 1.0f) * 255.0f)
        );
    }

    uint64_t toUInt64() const {
        return (
            (static_cast<uint64_t>(std::clamp(value.r, 0.0f, 1.0f) * 65535.0f) << 48) |
            (static_cast<uint64_t>(std::clamp(value.g, 0.0f, 1.0f) * 65535.0f) << 32) |
            (static_cast<uint64_t>(std::clamp(value.b, 0.0f, 1.0f) * 65535.0f) << 16) |
            static_cast<uint64_t>(std::clamp(value.a, 0.0f, 1.0f) * 65535.0f)
        );
    }

    uint64_t toUInt64PreMultipliedAlpha() const {
        glm::vec4 preMultiplied = {
            value.r * value.a,
            value.g * value.a,
            value.b * value.a,
            value.a
        };

        return (
            (static_cast<uint64_t>(std::clamp(preMultiplied.r, 0.0f, 1.0f) * 65535.0f) << 48) |
            (static_cast<uint64_t>(std::clamp(preMultiplied.g, 0.0f, 1.0f) * 65535.0f) << 32) |
            (static_cast<uint64_t>(std::clamp(preMultiplied.b, 0.0f, 1.0f) * 65535.0f) << 16) |
            static_cast<uint64_t>(std::clamp(preMultiplied.a, 0.0f, 1.0f) * 65535.0f)
        );
    }

    Color<COLORSPACE, true> toLinear() const {
        if constexpr (LINEAR) {
            return *this;
        } else {
            return {{
                Color::toLinear(value.r),
                Color::toLinear(value.g),
                Color::toLinear(value.b),
                value.a
            }};
        }
    }

    Color<COLORSPACE, false> toGamma() const {
        if constexpr (LINEAR) {
            return {{
                Color::toGamma(value.r),
                Color::toGamma(value.g),
                Color::toGamma(value.b),
                value.a
            }};
        } else {
            return *this;
        }
    }

    glm::vec4 transform(glm::mat3x3 mat) const {
        auto rgb = value.rgb();
        auto rgb_ = rgb * mat;
        return {rgb_, value.a};
    }

    Color<ColorSpace::XYZ, true> toXYZ() const {
        auto tmpLinear = toLinear();
        if constexpr (COLORSPACE == ColorSpace::sRGB) {
            return {tmpLinear.transform(matrix_sRGB_to_XYZ)};
        } else if constexpr (COLORSPACE == ColorSpace::XYZ) {
            return *this;
        }
    }

    Color composit(Color const &over, glm::vec3 subpixelMask) const {
        let overAlpha = subpixelMask * over.a();
        let underAlpha = glm::vec3{this->a(), this->a(), this->a()};
        let underAlpha_ = underAlpha * (glm::vec3{ 1.0, 1.0, 1.0 } - overAlpha);
        let alpha = overAlpha + underAlpha_;
        let color = (over.rgb() * overAlpha + this->rgb() * underAlpha_) / alpha;
        let averageAlpha = (alpha.r + alpha.g + alpha.b) / 3.0f;
        return { glm::vec4{color, averageAlpha} };
    }

    static double toLinear(double x) {
        if constexpr (COLORSPACE == ColorSpace::sRGB) {
            return x <= 0.04045 ? (x / 12.92) : pow((x + 0.055) / 1.055, 2.4);
        } else if constexpr (COLORSPACE == ColorSpace::XYZ) {
            case ColorSpace::XYZ: return x;
        }
    }

    static double toGamma(double x) {
        if constexpr (COLORSPACE == ColorSpace::sRGB) {
            return x <= 0.0031308 ? (x * 12.92) : (pow(x, 1.0 / 2.4) * 1.055) - 0.055;
        } else if constexpr (COLORSPACE == ColorSpace::XYZ) {
            return x;
        }
    }
};

using Color_sRGB = Color<ColorSpace::sRGB, false>;
using Color_sRGBLinear = Color<ColorSpace::sRGB, true>;
using Color_XYZ = Color<ColorSpace::XYZ, true>;

template<ColorSpace TO_COLORSPACE, bool TO_LINEAR, typename U>
Color<TO_COLORSPACE, TO_LINEAR> colorspace_cast(U from)
{
    if constexpr (TO_COLORSPACE == U::colorSpace) {
        if constexpr (TO_LINEAR == U::linear) {
            return from;
        } else if constexpr (TO_LINEAR) {
            return from.toLinear();
        } else {
            return from.toGamma();
        }

    } else {
        auto tmpXYZ = from.toXYZ();

        Color<TO_COLORSPACE, true> tmpLinear;
        if constexpr (TO_COLORSPACE == ColorSpace::sRGB) {
            tmpLinear = {tmpXYZ.transform(matrix_XYZ_to_sRGB)};
        } else if constexpr (TO_COLORSPACE == ColorSpace::XYZ) {
            tmpLinear = tmpXYZ;
        }

        if constexpr (TO_LINEAR) {
            return tmpLinear;
        } else {
            return tmpLinear.toGamma();
        }
    }
}

template<typename T, typename U>
T color_cast(U from) {
    return colorspace_cast<T::colorSpace, T::linear>(from);
}


}

