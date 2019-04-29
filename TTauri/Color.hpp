
#pragma once

namespace TTauri {

enum class ColorSpace {
    sRGB,
    XYZ
};

template<ColorSpace COLORSPACE=ColorSpace::sRGB, bool LINEAR=false>
struct Color {
    static const ColorSpace colorSpace = COLORSPACE;
    static const bool linear = LINEAR;

    glm::vec4 value;

    Color(glm::vec4 value) : value(value) {}

    /*! Convert a uint32_t value to a colour.
     * The uint32_t value is split into 4 bytes. Most-to-leas significant byte Red, Green, Blue, Alpha component.
     */
    Color(uint32_t rgba) {
        value = {
            static_cast<float>((v >> 24) & 0xff) * 0.00392156862745098,
            static_cast<float>((v >> 16) & 0xff) * 0.00392156862745098,
            static_cast<float>((v >> 8) & 0xff) * 0.00392156862745098,
            static_cast<float>(v & 0xff) * 0.00392156862745098,
        }
    }

    template<ColorSpace TO_COLORSPACE=ColorSpace::sRGB, bool TO_LINEAR=true>
    uint32_t toRGBA() {
        auto tmp = (*this)<TO_COLORSPACE, TO_LINEAR>();
        return (
            static_cast<uint32_t>(std::clamp(tmp.r, 0.0, 1.0) * 255.0) << 24,
            static_cast<uint32_t>(std::clamp(tmp.g, 0.0, 1.0) * 255.0) << 16,
            static_cast<uint32_t>(std::clamp(tmp.b, 0.0, 1.0) * 255.0) << 8,
            static_cast<uint32_t>(std::clamp(tmp.a, 0.0, 1.0) * 255.0)
        );
    }

    Color operator()(ColorSpace colorSpace=ColorSpace::sRGB) {
        if (this->colorSpace == colorSpace) {
            return *this;

        } else if (this->colorSpace == ColorSpace::XYZLinear) {

        } else if (colorSpace == ColorSpace::XYZLinear) {

        } else {
            auto tmp = (*this)(ColorSpace::XYZLinear);
            return tmp(colorSpace);
        }

    }
};


}

