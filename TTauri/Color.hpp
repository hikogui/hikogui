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

    Color(glm::vec4 value) : value(value) {}

    /*! Convert a uint32_t value to a colour.
     * The uint32_t value is split into 4 bytes. Most-to-leas significant byte Red, Green, Blue, Alpha component.
     */
    Color(uint32_t rgba) :
        value({
            static_cast<float>((rgba >> 24) & 0xff) / 255.0,
            static_cast<float>((rgba >> 16) & 0xff) / 255.0,
            static_cast<float>((rgba >> 8) & 0xff) / 255.0,
            static_cast<float>(rgba & 0xff) / 255.0
        }) {}

    float r() const { return value.r; }
    float g() const { return value.g; }
    float b() const { return value.b; }
    float a() const { return value.a; }

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
                Color<COLORSPACE>::toLinear(value.r),
                Color<COLORSPACE>::toLinear(value.g),
                Color<COLORSPACE>::toLinear(value.b),
                value.a
            }};
        }
    }

    Color<COLORSPACE, false> toGamma() const {
        if constexpr (LINEAR) {
            return {{
                Color<COLORSPACE>::toGamma(value.r),
                Color<COLORSPACE>::toGamma(value.g),
                Color<COLORSPACE>::toGamma(value.b),
                value.a
            }};
        } else {
            return *this;
        }
    }

    Color<ColorSpace::XYZ, true> toXYZ() const {
        auto tmpLinear = toLinear();
        switch (COLORSPACE) {
        case ColorSpace::sRGB: return {tmpLinear.value * matrix_sRGB_to_XYZ};
        case ColorSpace::XYZ: return *this;
        }
    }

    static float toLinear(float x) {
        switch constexpr (COLORSPACE) {
        case ColorSpace::sRGB: return x <= 0.04045 ? (x / 12.92) : powf((x + 0.055) / 1.055, 2.4);
        case ColorSpace::XYZ: return x;
        }
    }

    static float toGamma(float x) {
        switch constexpr (COLORSPACE) {
        case ColorSpace::sRGB: return u <= 0.0031308 ? (x * 12.92) : (powf(x, 1.0 / 2.4) * 1.055) - 0.055;
        case ColorSpace::XYZ: return x;
        }
    }
};

template<typename T, ColorSpace TO_COLORSPACE, bool TO_LINEAR>
Color<TO_COLORSPACE, TO_LINEAR> color_cast(T from)
{
    if constexpr (TO_COLORSPACE == from.colorSpace) {
        if constexpr (TO_LINEAR == from.linear) {
            return from;
        } else if constexpr (TO_LINEAR) {
            return from.toLinear();
        } else {
            return from.toGamma();
        }

    } else {
        auto tmpXYZ = from.toXYZ();

        Color<TO_COLORSPACE, true> tmpLinear;
        switch constexpr (TO_COLORSPACE) {
        case ColorSpace::sRGB: tmpLinear = {tmpXYZ.value * matrix_XYZ_to_sRGB};
        case ColorSpace::XYZ: tmpLinear = tmpXYZ;
        }

        if constexpr (TO_LINEAR) {
            return tmpLinear;
        } else {
            return tmpLinear.toGamma();
        }
    }
}

using Color_sRGB = Color<ColorSpace::sRGB, false>;

}

