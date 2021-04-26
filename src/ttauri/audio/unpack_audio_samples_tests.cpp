// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unpack_audio_samples.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace tt;

[[nodiscard]] constexpr std::array<std::byte, 256> make_packed_samples() noexcept
{
    auto r = std::array<std::byte, 256>{};
    for (size_t i = 0; i != 256; ++i) {
        if ((i / 2) % 2 == 0) {
            r[i] = static_cast<std::byte>(i);
        } else {
            r[i] = static_cast<std::byte>(255 - i);
        }
    }
    return r;
}

float int16_to_float(std::byte hi, std::byte lo)
{
    int16_t i = static_cast<int16_t>(hi) << 8 | static_cast<int16_t>(lo);
    return static_cast<float>(i) / 32767.0f;
}
TEST(unpack_audio_samples, unpack_int16le_mono)
{
    ttlet packed_samples = make_packed_samples();
    auto flat_samples = std::array<float,256>{};

    ttlet unpacker = unpack_audio_samples{2, 0, 16, false, std::endian::little, 2};

    unpacker(packed_samples.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed_samples[1], packed_samples[0]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed_samples.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed_samples[1], packed_samples[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed_samples[3], packed_samples[2]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed_samples.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed_samples[1], packed_samples[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed_samples[3], packed_samples[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed_samples[5], packed_samples[4]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed_samples.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed_samples[1], packed_samples[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed_samples[3], packed_samples[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed_samples[5], packed_samples[4]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed_samples[7], packed_samples[6]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed_samples.data(), flat_samples.data(), 5);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed_samples[1], packed_samples[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed_samples[3], packed_samples[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed_samples[5], packed_samples[4]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed_samples[7], packed_samples[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed_samples[9], packed_samples[8]), 0.000001f);
}
