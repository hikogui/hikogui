// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_sample_unpacker.hpp"
#include "../endian.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace hi;

constexpr float int16_max_diff = 1.0f / 32767.0f;
constexpr float int20_max_diff = 1.0f / 524287.0f;
constexpr float int24_max_diff = 1.0f / 8388607.0f;
constexpr float fix8_24_max_diff = int24_max_diff * 256.0f;
constexpr float float32_max_diff = 0.0f;

[[nodiscard]] static std::array<std::byte, 256> make_packed() noexcept
{
    auto r = std::array<std::byte, 256>{};
    for (std::size_t i = 0; i != 256; ++i) {
        if ((i / 2) % 2 == 0) {
            r[i] = static_cast<std::byte>(i);
        } else {
            r[i] = static_cast<std::byte>(255 - i);
        }
    }
    return r;
}

[[nodiscard]] static float int16_to_float(std::byte hi, std::byte lo)
{
    int16_t i = static_cast<int16_t>(hi) << 8 | static_cast<int16_t>(lo);
    return static_cast<float>(i) / 32767.0f;
}

[[nodiscard]] static float int24_to_float(std::byte hi, std::byte mid, std::byte lo)
{
    int32_t i = static_cast<int32_t>(hi) << 24 | static_cast<int32_t>(mid) << 16 | static_cast<int32_t>(lo) << 8;
    return static_cast<float>(i) / 2147483392.0f;
}

[[nodiscard]] static float int20_to_float(std::byte hi, std::byte mid, std::byte lo)
{
    int32_t i = static_cast<int32_t>(hi) << 24 | static_cast<int32_t>(mid) << 16 | static_cast<int32_t>(lo) << 8;
    return static_cast<float>(i) / 2147479552.0f;
}

[[nodiscard]] static float fix8_24_to_float(std::byte hi, std::byte mid_hi, std::byte mid_lo, std::byte lo)
{
    int32_t i = static_cast<int32_t>(hi) << 24 | static_cast<int32_t>(mid_hi) << 16 | static_cast<int32_t>(mid_lo) << 8 |
        static_cast<int32_t>(mid_lo);
    return static_cast<float>(i) / 8388607.0f;
}

[[nodiscard]] static float float32_to_float(std::byte hi, std::byte mid_hi, std::byte mid_lo, std::byte lo)
{
    uint32_t u = static_cast<uint32_t>(hi) << 24 | static_cast<uint32_t>(mid_hi) << 16 | static_cast<uint32_t>(mid_lo) << 8 |
        static_cast<uint32_t>(lo);

    // `u` was assembled as little-endian, so swap if that is not the native byte order.
    if (std::endian::little != std::endian::native) {
        u = byte_swap(u);
    }
    return std::bit_cast<float>(u);
}

TEST(audio_sample_unpacker, unpack_int16le_mono)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::int16_le(), 2};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[3], packed[2]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[3], packed[2]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[5], packed[4]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[3], packed[2]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[5], packed[4]), int16_max_diff);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[7], packed[6]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[3], packed[2]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[5], packed[4]), int16_max_diff);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[7], packed[6]), int16_max_diff);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed[9], packed[8]), int16_max_diff);
    ASSERT_NEAR(flat_samples[5], int16_to_float(packed[11], packed[10]), int16_max_diff);
    ASSERT_NEAR(flat_samples[6], int16_to_float(packed[13], packed[12]), int16_max_diff);
    ASSERT_NEAR(flat_samples[7], int16_to_float(packed[15], packed[14]), int16_max_diff);
}

TEST(audio_sample_unpacker, unpack_int16be_mono)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::int16_be(), 2};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[0], packed[1]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[0], packed[1]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[2], packed[3]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[0], packed[1]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[2], packed[3]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[4], packed[5]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[0], packed[1]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[2], packed[3]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[4], packed[5]), int16_max_diff);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[6], packed[7]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[0], packed[1]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[2], packed[3]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[4], packed[5]), int16_max_diff);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[6], packed[7]), int16_max_diff);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed[8], packed[9]), int16_max_diff);
    ASSERT_NEAR(flat_samples[5], int16_to_float(packed[10], packed[11]), int16_max_diff);
    ASSERT_NEAR(flat_samples[6], int16_to_float(packed[12], packed[13]), int16_max_diff);
    ASSERT_NEAR(flat_samples[7], int16_to_float(packed[14], packed[15]), int16_max_diff);
}

TEST(audio_sample_unpacker, unpack_int16le_stereo)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::int16_le(), 4};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[5], packed[4]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[5], packed[4]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[9], packed[8]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[5], packed[4]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[9], packed[8]), int16_max_diff);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[13], packed[12]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[5], packed[4]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[9], packed[8]), int16_max_diff);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[13], packed[12]), int16_max_diff);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed[17], packed[16]), int16_max_diff);
    ASSERT_NEAR(flat_samples[5], int16_to_float(packed[21], packed[20]), int16_max_diff);
    ASSERT_NEAR(flat_samples[6], int16_to_float(packed[25], packed[24]), int16_max_diff);
    ASSERT_NEAR(flat_samples[7], int16_to_float(packed[29], packed[28]), int16_max_diff);
}

TEST(audio_sample_unpacker, unpack_int16le_trio)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::int16_le(), 6};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[7], packed[6]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[7], packed[6]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[13], packed[12]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[7], packed[6]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[13], packed[12]), int16_max_diff);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[19], packed[18]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[7], packed[6]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[13], packed[12]), int16_max_diff);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[19], packed[18]), int16_max_diff);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed[25], packed[24]), int16_max_diff);
    ASSERT_NEAR(flat_samples[5], int16_to_float(packed[31], packed[30]), int16_max_diff);
    ASSERT_NEAR(flat_samples[6], int16_to_float(packed[37], packed[36]), int16_max_diff);
    ASSERT_NEAR(flat_samples[7], int16_to_float(packed[43], packed[42]), int16_max_diff);
}

TEST(audio_sample_unpacker, unpack_int16le_quadro)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::int16_le(), 8};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[9], packed[8]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[9], packed[8]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[17], packed[16]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[9], packed[8]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[17], packed[16]), int16_max_diff);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[25], packed[24]), int16_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), int16_max_diff);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[9], packed[8]), int16_max_diff);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[17], packed[16]), int16_max_diff);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[25], packed[24]), int16_max_diff);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed[33], packed[32]), int16_max_diff);
    ASSERT_NEAR(flat_samples[5], int16_to_float(packed[41], packed[40]), int16_max_diff);
    ASSERT_NEAR(flat_samples[6], int16_to_float(packed[49], packed[48]), int16_max_diff);
    ASSERT_NEAR(flat_samples[7], int16_to_float(packed[57], packed[56]), int16_max_diff);
}

TEST(audio_sample_unpacker, unpack_int24le_mono)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::int24_le(), 3};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[5], packed[4], packed[3]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[5], packed[4], packed[3]), int24_max_diff);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[8], packed[7], packed[6]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[5], packed[4], packed[3]), int24_max_diff);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[8], packed[7], packed[6]), int24_max_diff);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[11], packed[10], packed[9]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[5], packed[4], packed[3]), int24_max_diff);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[8], packed[7], packed[6]), int24_max_diff);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[11], packed[10], packed[9]), int24_max_diff);
    ASSERT_NEAR(flat_samples[4], int24_to_float(packed[14], packed[13], packed[12]), int24_max_diff);
    ASSERT_NEAR(flat_samples[5], int24_to_float(packed[17], packed[16], packed[15]), int24_max_diff);
    ASSERT_NEAR(flat_samples[6], int24_to_float(packed[20], packed[19], packed[18]), int24_max_diff);
    ASSERT_NEAR(flat_samples[7], int24_to_float(packed[23], packed[22], packed[21]), int24_max_diff);
}

TEST(audio_sample_unpacker, unpack_int24be_mono)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::int24_be(), 3};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[0], packed[1], packed[2]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[0], packed[1], packed[2]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[3], packed[4], packed[5]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[0], packed[1], packed[2]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[3], packed[4], packed[5]), int24_max_diff);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[6], packed[7], packed[8]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[0], packed[1], packed[2]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[3], packed[4], packed[5]), int24_max_diff);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[6], packed[7], packed[8]), int24_max_diff);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[9], packed[10], packed[11]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[0], packed[1], packed[2]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[3], packed[4], packed[5]), int24_max_diff);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[6], packed[7], packed[8]), int24_max_diff);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[9], packed[10], packed[11]), int24_max_diff);
    ASSERT_NEAR(flat_samples[4], int24_to_float(packed[12], packed[13], packed[14]), int24_max_diff);
    ASSERT_NEAR(flat_samples[5], int24_to_float(packed[15], packed[16], packed[17]), int24_max_diff);
    ASSERT_NEAR(flat_samples[6], int24_to_float(packed[18], packed[19], packed[20]), int24_max_diff);
    ASSERT_NEAR(flat_samples[7], int24_to_float(packed[21], packed[22], packed[23]), int24_max_diff);
}

TEST(audio_sample_unpacker, unpack_int24le_stereo)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::int24_le(), 6};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[8], packed[7], packed[6]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[8], packed[7], packed[6]), int24_max_diff);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[14], packed[13], packed[12]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[8], packed[7], packed[6]), int24_max_diff);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[14], packed[13], packed[12]), int24_max_diff);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[20], packed[19], packed[18]), int24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), int24_max_diff);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[8], packed[7], packed[6]), int24_max_diff);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[14], packed[13], packed[12]), int24_max_diff);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[20], packed[19], packed[18]), int24_max_diff);
    ASSERT_NEAR(flat_samples[4], int24_to_float(packed[26], packed[25], packed[24]), int24_max_diff);
    ASSERT_NEAR(flat_samples[5], int24_to_float(packed[32], packed[31], packed[30]), int24_max_diff);
    ASSERT_NEAR(flat_samples[6], int24_to_float(packed[38], packed[37], packed[36]), int24_max_diff);
    ASSERT_NEAR(flat_samples[7], int24_to_float(packed[44], packed[43], packed[42]), int24_max_diff);
}

TEST(audio_sample_unpacker, unpack_int20le_mono)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::int20_le(), 3};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[2], packed[1], packed[0]), int20_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[2], packed[1], packed[0]), int20_max_diff);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[5], packed[4], packed[3]), int20_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[2], packed[1], packed[0]), int20_max_diff);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[5], packed[4], packed[3]), int20_max_diff);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[8], packed[7], packed[6]), int20_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[2], packed[1], packed[0]), int20_max_diff);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[5], packed[4], packed[3]), int20_max_diff);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[8], packed[7], packed[6]), int20_max_diff);
    ASSERT_NEAR(flat_samples[3], int20_to_float(packed[11], packed[10], packed[9]), int20_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[2], packed[1], packed[0]), int20_max_diff);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[5], packed[4], packed[3]), int20_max_diff);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[8], packed[7], packed[6]), int20_max_diff);
    ASSERT_NEAR(flat_samples[3], int20_to_float(packed[11], packed[10], packed[9]), int20_max_diff);
    ASSERT_NEAR(flat_samples[4], int20_to_float(packed[14], packed[13], packed[12]), int20_max_diff);
    ASSERT_NEAR(flat_samples[5], int20_to_float(packed[17], packed[16], packed[15]), int20_max_diff);
    ASSERT_NEAR(flat_samples[6], int20_to_float(packed[20], packed[19], packed[18]), int20_max_diff);
    ASSERT_NEAR(flat_samples[7], int20_to_float(packed[23], packed[22], packed[21]), int20_max_diff);
}

TEST(audio_sample_unpacker, unpack_int20be_mono)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::int20_be(), 3};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[0], packed[1], packed[2]), int20_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[0], packed[1], packed[2]), int20_max_diff);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[3], packed[4], packed[5]), int20_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[0], packed[1], packed[2]), int20_max_diff);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[3], packed[4], packed[5]), int20_max_diff);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[6], packed[7], packed[8]), int20_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[0], packed[1], packed[2]), int20_max_diff);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[3], packed[4], packed[5]), int20_max_diff);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[6], packed[7], packed[8]), int20_max_diff);
    ASSERT_NEAR(flat_samples[3], int20_to_float(packed[9], packed[10], packed[11]), int20_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[0], packed[1], packed[2]), int20_max_diff);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[3], packed[4], packed[5]), int20_max_diff);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[6], packed[7], packed[8]), int20_max_diff);
    ASSERT_NEAR(flat_samples[3], int20_to_float(packed[9], packed[10], packed[11]), int20_max_diff);
    ASSERT_NEAR(flat_samples[4], int20_to_float(packed[12], packed[13], packed[14]), int20_max_diff);
    ASSERT_NEAR(flat_samples[5], int20_to_float(packed[15], packed[16], packed[17]), int20_max_diff);
    ASSERT_NEAR(flat_samples[6], int20_to_float(packed[18], packed[19], packed[20]), int20_max_diff);
    ASSERT_NEAR(flat_samples[7], int20_to_float(packed[21], packed[22], packed[23]), int20_max_diff);
}

TEST(audio_sample_unpacker, unpack_fix8_24le_mono)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::fix8_23_le(), 4};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[3], packed[2], packed[1], packed[0]), fix8_24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[3], packed[2], packed[1], packed[0]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[7], packed[6], packed[5], packed[4]), fix8_24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[3], packed[2], packed[1], packed[0]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[7], packed[6], packed[5], packed[4]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[11], packed[10], packed[9], packed[8]), fix8_24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[3], packed[2], packed[1], packed[0]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[7], packed[6], packed[5], packed[4]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[11], packed[10], packed[9], packed[8]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[3], fix8_24_to_float(packed[15], packed[14], packed[13], packed[12]), fix8_24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[3], packed[2], packed[1], packed[0]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[7], packed[6], packed[5], packed[4]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[11], packed[10], packed[9], packed[8]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[3], fix8_24_to_float(packed[15], packed[14], packed[13], packed[12]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[4], fix8_24_to_float(packed[19], packed[18], packed[17], packed[16]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[5], fix8_24_to_float(packed[23], packed[22], packed[21], packed[20]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[6], fix8_24_to_float(packed[27], packed[26], packed[25], packed[24]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[7], fix8_24_to_float(packed[31], packed[30], packed[29], packed[28]), fix8_24_max_diff);
}

TEST(audio_sample_unpacker, unpack_fix8_24be_mono)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::fix8_23_be(), 4};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[0], packed[1], packed[2], packed[3]), fix8_24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[0], packed[1], packed[2], packed[3]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[4], packed[5], packed[6], packed[7]), fix8_24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[0], packed[1], packed[2], packed[3]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[4], packed[5], packed[6], packed[7]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[8], packed[9], packed[10], packed[11]), fix8_24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[0], packed[1], packed[2], packed[3]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[4], packed[5], packed[6], packed[7]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[8], packed[9], packed[10], packed[11]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[3], fix8_24_to_float(packed[12], packed[13], packed[14], packed[15]), fix8_24_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[0], packed[1], packed[2], packed[3]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[4], packed[5], packed[6], packed[7]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[8], packed[9], packed[10], packed[11]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[3], fix8_24_to_float(packed[12], packed[13], packed[14], packed[15]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[4], fix8_24_to_float(packed[16], packed[17], packed[18], packed[19]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[5], fix8_24_to_float(packed[20], packed[21], packed[22], packed[23]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[6], fix8_24_to_float(packed[24], packed[25], packed[26], packed[27]), fix8_24_max_diff);
    ASSERT_NEAR(flat_samples[7], fix8_24_to_float(packed[28], packed[29], packed[30], packed[31]), fix8_24_max_diff);
}

TEST(audio_sample_unpacker, unpack_float32le_mono)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::float32_le(), 4};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[3], packed[2], packed[1], packed[0]), float32_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[3], packed[2], packed[1], packed[0]), float32_max_diff);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[7], packed[6], packed[5], packed[4]), float32_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[3], packed[2], packed[1], packed[0]), float32_max_diff);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[7], packed[6], packed[5], packed[4]), float32_max_diff);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[11], packed[10], packed[9], packed[8]), float32_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[3], packed[2], packed[1], packed[0]), float32_max_diff);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[7], packed[6], packed[5], packed[4]), float32_max_diff);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[11], packed[10], packed[9], packed[8]), float32_max_diff);
    ASSERT_NEAR(flat_samples[3], float32_to_float(packed[15], packed[14], packed[13], packed[12]), float32_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[3], packed[2], packed[1], packed[0]), float32_max_diff);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[7], packed[6], packed[5], packed[4]), float32_max_diff);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[11], packed[10], packed[9], packed[8]), float32_max_diff);
    ASSERT_NEAR(flat_samples[3], float32_to_float(packed[15], packed[14], packed[13], packed[12]), float32_max_diff);
    ASSERT_NEAR(flat_samples[4], float32_to_float(packed[19], packed[18], packed[17], packed[16]), float32_max_diff);
    ASSERT_NEAR(flat_samples[5], float32_to_float(packed[23], packed[22], packed[21], packed[20]), float32_max_diff);
    ASSERT_NEAR(flat_samples[6], float32_to_float(packed[27], packed[26], packed[25], packed[24]), float32_max_diff);
    ASSERT_NEAR(flat_samples[7], float32_to_float(packed[31], packed[30], packed[29], packed[28]), float32_max_diff);
}

TEST(audio_sample_unpacker, unpack_float32be_mono)
{
    hilet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};
    hilet unpacker = audio_sample_unpacker{audio_sample_format::float32_be(), 4};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[0], packed[1], packed[2], packed[3]), float32_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[0], packed[1], packed[2], packed[3]), float32_max_diff);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[4], packed[5], packed[6], packed[7]), float32_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[0], packed[1], packed[2], packed[3]), float32_max_diff);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[4], packed[5], packed[6], packed[7]), float32_max_diff);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[8], packed[9], packed[10], packed[11]), float32_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[0], packed[1], packed[2], packed[3]), float32_max_diff);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[4], packed[5], packed[6], packed[7]), float32_max_diff);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[8], packed[9], packed[10], packed[11]), float32_max_diff);
    ASSERT_NEAR(flat_samples[3], float32_to_float(packed[12], packed[13], packed[14], packed[15]), float32_max_diff);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[0], packed[1], packed[2], packed[3]), float32_max_diff);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[4], packed[5], packed[6], packed[7]), float32_max_diff);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[8], packed[9], packed[10], packed[11]), float32_max_diff);
    ASSERT_NEAR(flat_samples[3], float32_to_float(packed[12], packed[13], packed[14], packed[15]), float32_max_diff);
    ASSERT_NEAR(flat_samples[4], float32_to_float(packed[16], packed[17], packed[18], packed[19]), float32_max_diff);
    ASSERT_NEAR(flat_samples[5], float32_to_float(packed[20], packed[21], packed[22], packed[23]), float32_max_diff);
    ASSERT_NEAR(flat_samples[6], float32_to_float(packed[24], packed[25], packed[26], packed[27]), float32_max_diff);
    ASSERT_NEAR(flat_samples[7], float32_to_float(packed[28], packed[29], packed[30], packed[31]), float32_max_diff);
}
