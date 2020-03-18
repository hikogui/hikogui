// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/float16.hpp"
#include <cmath>

namespace TTauri {

[[nodiscard]] inline float sRGB_linear_to_gamma(float u) noexcept
{
    if (u <= 0.0031308) {
        return 12.92f * u;
    } else {
        return 1.055f * pow(u, 0.416f) - 0.055f;
    }
}

[[nodiscard]] inline auto sRGB_linear16_to_gamma8_table_generator() noexcept
{
    std::array<uint8_t,65536> r{};

    for (int i = 0; i != 65536; ++i) {
        r[i] = static_cast<uint8_t>(
            std::clamp(sRGB_linear_to_gamma(float16{i, true}), 0.0f, 1.0f) * 255.0f
        );
    }

    return r;
}

inline auto sRGB_linear16_to_gamma8_table = sRGB_linear16_to_gamma8_table_generator();

[[nodiscard]] inline uint8_t sRGB_linear16_to_gamma8(float16 u) noexcept
{
    return sRGB_linear16_to_gamma8_table[u.get()];
}

[[nodiscard]] inline float sRGB_gamma_to_linear(float u) noexcept
{
    if (u <= 0.04045) {
        return u / 12.92f;
    } else {
        return pow((u + 0.055f) / 1.055f, 2.4f);
    }
}

[[nodiscard]] inline auto sRGB_gamma8_to_linear16_table_generator() noexcept
{
    std::array<float16,256> r{};

    for (int i = 0; i != 256; ++i) {
        r[i] = static_cast<float16_t>(sRGB_gamma8_to_linear16(i / 255.0f));
    }

    return r;
}

inline auto sRGB_gamma8_to_linear16_table = sRGB_gamma8_to_linear16_table_generator();

[[nodiscard]] inline float16 sRGB_gamma8_to_linear16(uint8_t u) noexcept
{
    return sRGB_gamma8_to_linear16_table[u];
}


}

