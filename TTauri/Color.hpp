// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <glm/glm.hpp>
#include <boost/format.hpp>
#include <string>

namespace TTauri {

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
        uint32_t tmp = toRGBA();
        return (boost::format("#%08x") % tmp).str();
    }

    uint32_t toRGBA() const {
        return (
            (static_cast<uint32_t>(std::clamp(value.r, 0.0f, 1.0f) * 255.0) << 24) |
            (static_cast<uint32_t>(std::clamp(value.g, 0.0f, 1.0f) * 255.0) << 16) |
            (static_cast<uint32_t>(std::clamp(value.b, 0.0f, 1.0f) * 255.0) << 8) |
            static_cast<uint32_t>(std::clamp(value.a, 0.0f, 1.0f) * 255.0)
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
        let underAlphaResult = overAlpha + underAlpha * (glm::vec3{1.0, 1.0, 1.0} - overAlpha);
        let color = over.rgb() * overAlpha + this->rgb() * underAlphaResult;
        let alpha = overAlpha + underAlphaResult;
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

