// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/float16.hpp"
#include "TTauri/Foundation/mat.hpp"
#include <cmath>
#include <array>

namespace tt {

inline mat sRGB_to_XYZ = mat{
    0.41239080f, 0.35758434f, 0.18048079f, 0.0f,
    0.21263901f, 0.71516868f, 0.07219232f, 0.0f,
    0.01933082f, 0.11919478f, 0.95053215f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

inline mat XYZ_to_sRGB = mat{
     3.24096994f, -1.53738318f, -0.49861076f, 0.0f,
    -0.96924364f,  1.87596750f,  0.04155506f, 0.0f,
     0.05563008f, -0.20397696f,  1.05697151f, 0.0f,
     0.0f, 0.0f, 0.0f, 1.0f
};

[[nodiscard]] inline float sRGB_linear_to_gamma(float u) noexcept
{
    if (u <= 0.0031308) {
        return 12.92f * u;
    } else {
        return 1.055f * std::pow(u, 0.416f) - 0.055f;
    }
}

[[nodiscard]] inline float sRGB_gamma_to_linear(float u) noexcept
{
    if (u <= 0.04045) {
        return u / 12.92f;
    } else {
        return std::pow((u + 0.055f) / 1.055f, 2.4f);
    }
}

[[nodiscard]] inline auto sRGB_linear16_to_gamma8_table_generator() noexcept
{
    std::array<uint8_t,65536> r{};

    for (int i = 0; i != 65536; ++i) {
        r[i] = static_cast<uint8_t>(
            std::clamp(sRGB_linear_to_gamma(float16{numeric_cast<uint16_t>(i), true}), 0.0f, 1.0f) * 255.0f
        );
    }

    return r;
}

inline auto sRGB_linear16_to_gamma8_table = sRGB_linear16_to_gamma8_table_generator();

[[nodiscard]] inline uint8_t sRGB_linear16_to_gamma8(float16 u) noexcept
{
    return sRGB_linear16_to_gamma8_table[u.get()];
}



[[nodiscard]] inline auto sRGB_gamma8_to_linear16_table_generator() noexcept
{
    std::array<float16,256> r{};

    for (int i = 0; i != 256; ++i) {
        r[i] = static_cast<float16>(sRGB_gamma_to_linear(i / 255.0f));
    }

    return r;
}

inline auto sRGB_gamma8_to_linear16_table = sRGB_gamma8_to_linear16_table_generator();

[[nodiscard]] inline float16 sRGB_gamma8_to_linear16(uint8_t u) noexcept
{
    return sRGB_gamma8_to_linear16_table[u];
}


}

