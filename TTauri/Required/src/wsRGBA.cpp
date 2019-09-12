// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Required/wsRGBA.hpp"

namespace TTauri {

static float linear_to_gamma_f32(float u) noexcept
{
    if (u <= 0.0031308f) {
        return u * 12.92f;
    } else {
        return std::pow(u, 1.0f/2.4f) * 1.055f - 0.055f;
    }
}

static float gamma_to_linear_f32(float u) noexcept
{
    if (u <= 0.04045f) {
        return u / 12.92f;
    } else {
        return std::pow((u + 0.055f) / 1.055f, 2.4f);
    }
}

const std::array<int16_t,256> gamma_to_linear_i16_table = generate_array<int16_t, 256>([](auto i) {
    let u = i / 255.0f;
    return static_cast<int16_t>(gamma_to_linear_f32(u) * 4095.0f + 0.5f);
});


const std::array<uint8_t,4096> linear_to_gamma_u8_table = generate_array<uint8_t, 4096>([](auto i) {
    let u = i / 4095.0f;
    return static_cast<uint8_t>(linear_to_gamma_f32(u) * 255.0f + 0.5f);
});

}
