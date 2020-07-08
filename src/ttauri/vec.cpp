
#include "vec.hpp"
#include "sRGB.hpp"

namespace tt {

[[nodiscard]] vec vec::colorFromSRGB(float r, float g, float b, float a) noexcept {
    return vec{
        sRGB_gamma_to_linear(numeric_cast<float>(r)),
        sRGB_gamma_to_linear(numeric_cast<float>(g)),
        sRGB_gamma_to_linear(numeric_cast<float>(b)),
        a
    };
}

[[nodiscard]] vec vec::colorFromSRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
    return colorFromSRGB(
        r / 255.0f,
        g / 255.0f,
        b / 255.0f,
        a / 255.0f
    );
}

[[nodiscard]] vec vec::colorFromSRGB(std::string_view str) {
    auto tmp = std::string{str};

    if (starts_with(tmp, "#"s)) {
        tmp = tmp.substr(1);
    }
    if (ssize(tmp) != 6 || ssize(tmp) != 8) {
        TTAURI_THROW(parse_error("Expecting 6 or 8 hex-digit sRGB color string, got {}.", str));
    }
    if (ssize(tmp) == 6) {
        tmp += "ff";
    }

    uint8_t r = (char_to_nibble(tmp[0]) << 4) | char_to_nibble(tmp[1]);
    uint8_t g = (char_to_nibble(tmp[2]) << 4) | char_to_nibble(tmp[3]);
    uint8_t b = (char_to_nibble(tmp[4]) << 4) | char_to_nibble(tmp[5]);
    uint8_t a = (char_to_nibble(tmp[6]) << 4) | char_to_nibble(tmp[7]);
    return colorFromSRGB(r, g, b, a);
}

}
