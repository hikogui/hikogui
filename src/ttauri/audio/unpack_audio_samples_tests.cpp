// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unpack_audio_samples.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace tt;

[[nodiscard]] constexpr std::array<std::byte, 256> make_packed() noexcept
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

float int24_to_float(std::byte hi, std::byte mid, std::byte lo)
{
    int32_t i = static_cast<int32_t>(hi) << 24 | static_cast<int32_t>(mid) << 16 | static_cast<int32_t>(lo) << 8;
    return static_cast<float>(i) / 2147483392.0f;
}

float int20_to_float(std::byte hi, std::byte mid, std::byte lo)
{
    int32_t i = static_cast<int32_t>(hi) << 24 | static_cast<int32_t>(mid) << 16 | static_cast<int32_t>(lo) << 8;
    return static_cast<float>(i) / 2147479552.0f;
}

float fix8_24_to_float(std::byte hi, std::byte mid_hi, std::byte mid_lo, std::byte lo)
{
    int32_t i = static_cast<int32_t>(hi) << 24 | static_cast<int32_t>(mid_hi) << 16 | static_cast<int32_t>(mid_lo) << 8 |
        static_cast<int32_t>(mid_lo);
    return static_cast<float>(i) / 8388607.0f;
}

float float32_to_float(std::byte hi, std::byte mid_hi, std::byte mid_lo, std::byte lo)
{
    uint32_t u = static_cast<uint32_t>(hi) << 24 | static_cast<uint32_t>(mid_hi) << 16 | static_cast<uint32_t>(mid_lo) << 8 |
        static_cast<uint32_t>(lo);

    // `u` was assembled as little-endian, so swap if that is not the native byte order.
    if (std::endian::little != std::endian::native) {
        u = byte_swap(u);
    }
    return std::bit_cast<float>(u);
}

TEST(unpack_audio_samples, unpack_int16le_mono)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{2, 0, 16, false, std::endian::little, 2};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[3], packed[2]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[3], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[5], packed[4]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[3], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[5], packed[4]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[7], packed[6]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[3], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[5], packed[4]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed[9], packed[8]), 0.000001f);
    ASSERT_NEAR(flat_samples[5], int16_to_float(packed[11], packed[10]), 0.000001f);
    ASSERT_NEAR(flat_samples[6], int16_to_float(packed[13], packed[12]), 0.000001f);
    ASSERT_NEAR(flat_samples[7], int16_to_float(packed[15], packed[14]), 0.000001f);
}

TEST(unpack_audio_samples, unpack_int16be_mono)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{2, 0, 16, false, std::endian::big, 2};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[0], packed[1]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[0], packed[1]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[2], packed[3]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[0], packed[1]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[2], packed[3]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[4], packed[5]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[0], packed[1]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[2], packed[3]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[4], packed[5]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[6], packed[7]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[0], packed[1]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[2], packed[3]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[4], packed[5]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[6], packed[7]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed[8], packed[9]), 0.000001f);
    ASSERT_NEAR(flat_samples[5], int16_to_float(packed[10], packed[11]), 0.000001f);
    ASSERT_NEAR(flat_samples[6], int16_to_float(packed[12], packed[13]), 0.000001f);
    ASSERT_NEAR(flat_samples[7], int16_to_float(packed[14], packed[15]), 0.000001f);
}

TEST(unpack_audio_samples, unpack_int16le_stereo)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{2, 0, 16, false, std::endian::little, 4};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[5], packed[4]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[5], packed[4]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[9], packed[8]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[5], packed[4]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[9], packed[8]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[13], packed[12]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[5], packed[4]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[9], packed[8]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[13], packed[12]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed[17], packed[16]), 0.000001f);
    ASSERT_NEAR(flat_samples[5], int16_to_float(packed[21], packed[20]), 0.000001f);
    ASSERT_NEAR(flat_samples[6], int16_to_float(packed[25], packed[24]), 0.000001f);
    ASSERT_NEAR(flat_samples[7], int16_to_float(packed[29], packed[28]), 0.000001f);
}

TEST(unpack_audio_samples, unpack_int16le_trio)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{2, 0, 16, false, std::endian::little, 6};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[7], packed[6]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[13], packed[12]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[13], packed[12]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[19], packed[18]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[13], packed[12]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[19], packed[18]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed[25], packed[24]), 0.000001f);
    ASSERT_NEAR(flat_samples[5], int16_to_float(packed[31], packed[30]), 0.000001f);
    ASSERT_NEAR(flat_samples[6], int16_to_float(packed[37], packed[36]), 0.000001f);
    ASSERT_NEAR(flat_samples[7], int16_to_float(packed[43], packed[42]), 0.000001f);
}

TEST(unpack_audio_samples, unpack_int16le_quadro)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{2, 0, 16, false, std::endian::little, 8};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[9], packed[8]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[9], packed[8]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[17], packed[16]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[9], packed[8]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[17], packed[16]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[25], packed[24]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int16_to_float(packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int16_to_float(packed[9], packed[8]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int16_to_float(packed[17], packed[16]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int16_to_float(packed[25], packed[24]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int16_to_float(packed[33], packed[32]), 0.000001f);
    ASSERT_NEAR(flat_samples[5], int16_to_float(packed[41], packed[40]), 0.000001f);
    ASSERT_NEAR(flat_samples[6], int16_to_float(packed[49], packed[48]), 0.000001f);
    ASSERT_NEAR(flat_samples[7], int16_to_float(packed[57], packed[56]), 0.000001f);
}

TEST(unpack_audio_samples, unpack_int24le_mono)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{3, 0, 24, false, std::endian::little, 3};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[5], packed[4], packed[3]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[5], packed[4], packed[3]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[8], packed[7], packed[6]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[5], packed[4], packed[3]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[8], packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[11], packed[10], packed[9]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[5], packed[4], packed[3]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[8], packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[11], packed[10], packed[9]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int24_to_float(packed[14], packed[13], packed[12]), 0.000001f);
    ASSERT_NEAR(flat_samples[5], int24_to_float(packed[17], packed[16], packed[15]), 0.000001f);
    ASSERT_NEAR(flat_samples[6], int24_to_float(packed[20], packed[19], packed[18]), 0.000001f);
    ASSERT_NEAR(flat_samples[7], int24_to_float(packed[23], packed[22], packed[21]), 0.000001f);
}

TEST(unpack_audio_samples, unpack_int24be_mono)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{3, 0, 24, false, std::endian::big, 3};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[0], packed[1], packed[2]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[0], packed[1], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[3], packed[4], packed[5]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[0], packed[1], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[3], packed[4], packed[5]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[6], packed[7], packed[8]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[0], packed[1], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[3], packed[4], packed[5]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[6], packed[7], packed[8]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[9], packed[10], packed[11]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[0], packed[1], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[3], packed[4], packed[5]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[6], packed[7], packed[8]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[9], packed[10], packed[11]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int24_to_float(packed[12], packed[13], packed[14]), 0.000001f);
    ASSERT_NEAR(flat_samples[5], int24_to_float(packed[15], packed[16], packed[17]), 0.000001f);
    ASSERT_NEAR(flat_samples[6], int24_to_float(packed[18], packed[19], packed[20]), 0.000001f);
    ASSERT_NEAR(flat_samples[7], int24_to_float(packed[21], packed[22], packed[23]), 0.000001f);
}

TEST(unpack_audio_samples, unpack_int24le_stereo)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{3, 0, 24, false, std::endian::little, 6};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[8], packed[7], packed[6]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[8], packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[14], packed[13], packed[12]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[8], packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[14], packed[13], packed[12]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[20], packed[19], packed[18]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int24_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int24_to_float(packed[8], packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int24_to_float(packed[14], packed[13], packed[12]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int24_to_float(packed[20], packed[19], packed[18]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int24_to_float(packed[26], packed[25], packed[24]), 0.000001f);
    ASSERT_NEAR(flat_samples[5], int24_to_float(packed[32], packed[31], packed[30]), 0.000001f);
    ASSERT_NEAR(flat_samples[6], int24_to_float(packed[38], packed[37], packed[36]), 0.000001f);
    ASSERT_NEAR(flat_samples[7], int24_to_float(packed[44], packed[43], packed[42]), 0.000001f);
}

TEST(unpack_audio_samples, unpack_int20le_mono)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{3, 0, 24, false, std::endian::little, 3};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[2], packed[1], packed[0]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[5], packed[4], packed[3]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[5], packed[4], packed[3]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[8], packed[7], packed[6]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[5], packed[4], packed[3]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[8], packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int20_to_float(packed[11], packed[10], packed[9]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[2], packed[1], packed[0]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[5], packed[4], packed[3]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[8], packed[7], packed[6]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int20_to_float(packed[11], packed[10], packed[9]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int20_to_float(packed[14], packed[13], packed[12]), 0.000001f);
    ASSERT_NEAR(flat_samples[5], int20_to_float(packed[17], packed[16], packed[15]), 0.000001f);
    ASSERT_NEAR(flat_samples[6], int20_to_float(packed[20], packed[19], packed[18]), 0.000001f);
    ASSERT_NEAR(flat_samples[7], int20_to_float(packed[23], packed[22], packed[21]), 0.000001f);
}

TEST(unpack_audio_samples, unpack_int20be_mono)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{3, 0, 24, false, std::endian::big, 3};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[0], packed[1], packed[2]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[0], packed[1], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[3], packed[4], packed[5]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[0], packed[1], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[3], packed[4], packed[5]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[6], packed[7], packed[8]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[0], packed[1], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[3], packed[4], packed[5]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[6], packed[7], packed[8]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int20_to_float(packed[9], packed[10], packed[11]), 0.000001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], int20_to_float(packed[0], packed[1], packed[2]), 0.000001f);
    ASSERT_NEAR(flat_samples[1], int20_to_float(packed[3], packed[4], packed[5]), 0.000001f);
    ASSERT_NEAR(flat_samples[2], int20_to_float(packed[6], packed[7], packed[8]), 0.000001f);
    ASSERT_NEAR(flat_samples[3], int20_to_float(packed[9], packed[10], packed[11]), 0.000001f);
    ASSERT_NEAR(flat_samples[4], int20_to_float(packed[12], packed[13], packed[14]), 0.000001f);
    ASSERT_NEAR(flat_samples[5], int20_to_float(packed[15], packed[16], packed[17]), 0.000001f);
    ASSERT_NEAR(flat_samples[6], int20_to_float(packed[18], packed[19], packed[20]), 0.000001f);
    ASSERT_NEAR(flat_samples[7], int20_to_float(packed[21], packed[22], packed[23]), 0.000001f);
}

TEST(unpack_audio_samples, unpack_fix8_24le_mono)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{4, 8, 24, false, std::endian::little, 4};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[3], packed[2], packed[1], packed[0]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[3], packed[2], packed[1], packed[0]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[7], packed[6], packed[5], packed[4]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[3], packed[2], packed[1], packed[0]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[7], packed[6], packed[5], packed[4]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[11], packed[10], packed[9], packed[8]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[3], packed[2], packed[1], packed[0]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[7], packed[6], packed[5], packed[4]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[11], packed[10], packed[9], packed[8]), 0.00001f);
    ASSERT_NEAR(flat_samples[3], fix8_24_to_float(packed[15], packed[14], packed[13], packed[12]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[3], packed[2], packed[1], packed[0]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[7], packed[6], packed[5], packed[4]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[11], packed[10], packed[9], packed[8]), 0.00001f);
    ASSERT_NEAR(flat_samples[3], fix8_24_to_float(packed[15], packed[14], packed[13], packed[12]), 0.00001f);
    ASSERT_NEAR(flat_samples[4], fix8_24_to_float(packed[19], packed[18], packed[17], packed[16]), 0.00001f);
    ASSERT_NEAR(flat_samples[5], fix8_24_to_float(packed[23], packed[22], packed[21], packed[20]), 0.00001f);
    ASSERT_NEAR(flat_samples[6], fix8_24_to_float(packed[27], packed[26], packed[25], packed[24]), 0.00001f);
    ASSERT_NEAR(flat_samples[7], fix8_24_to_float(packed[31], packed[30], packed[29], packed[28]), 0.00001f);
}

TEST(unpack_audio_samples, unpack_fix8_24be_mono)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{4, 8, 24, false, std::endian::big, 4};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[0], packed[1], packed[2], packed[3]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[0], packed[1], packed[2], packed[3]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[4], packed[5], packed[6], packed[7]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[0], packed[1], packed[2], packed[3]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[4], packed[5], packed[6], packed[7]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[8], packed[9], packed[10], packed[11]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[0], packed[1], packed[2], packed[3]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[4], packed[5], packed[6], packed[7]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[8], packed[9], packed[10], packed[11]), 0.00001f);
    ASSERT_NEAR(flat_samples[3], fix8_24_to_float(packed[12], packed[13], packed[14], packed[15]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], fix8_24_to_float(packed[0], packed[1], packed[2], packed[3]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], fix8_24_to_float(packed[4], packed[5], packed[6], packed[7]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], fix8_24_to_float(packed[8], packed[9], packed[10], packed[11]), 0.00001f);
    ASSERT_NEAR(flat_samples[3], fix8_24_to_float(packed[12], packed[13], packed[14], packed[15]), 0.00001f);
    ASSERT_NEAR(flat_samples[4], fix8_24_to_float(packed[16], packed[17], packed[18], packed[19]), 0.00001f);
    ASSERT_NEAR(flat_samples[5], fix8_24_to_float(packed[20], packed[21], packed[22], packed[23]), 0.00001f);
    ASSERT_NEAR(flat_samples[6], fix8_24_to_float(packed[24], packed[25], packed[26], packed[27]), 0.00001f);
    ASSERT_NEAR(flat_samples[7], fix8_24_to_float(packed[28], packed[29], packed[30], packed[31]), 0.00001f);
}

TEST(unpack_audio_samples, unpack_float32le_mono)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{4, 0, 32, true, std::endian::little, 4};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[3], packed[2], packed[1], packed[0]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[3], packed[2], packed[1], packed[0]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[7], packed[6], packed[5], packed[4]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[3], packed[2], packed[1], packed[0]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[7], packed[6], packed[5], packed[4]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[11], packed[10], packed[9], packed[8]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[3], packed[2], packed[1], packed[0]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[7], packed[6], packed[5], packed[4]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[11], packed[10], packed[9], packed[8]), 0.00001f);
    ASSERT_NEAR(flat_samples[3], float32_to_float(packed[15], packed[14], packed[13], packed[12]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[3], packed[2], packed[1], packed[0]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[7], packed[6], packed[5], packed[4]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[11], packed[10], packed[9], packed[8]), 0.00001f);
    ASSERT_NEAR(flat_samples[3], float32_to_float(packed[15], packed[14], packed[13], packed[12]), 0.00001f);
    ASSERT_NEAR(flat_samples[4], float32_to_float(packed[19], packed[18], packed[17], packed[16]), 0.00001f);
    ASSERT_NEAR(flat_samples[5], float32_to_float(packed[23], packed[22], packed[21], packed[20]), 0.00001f);
    ASSERT_NEAR(flat_samples[6], float32_to_float(packed[27], packed[26], packed[25], packed[24]), 0.00001f);
    ASSERT_NEAR(flat_samples[7], float32_to_float(packed[31], packed[30], packed[29], packed[28]), 0.00001f);
}

TEST(unpack_audio_samples, unpack_float32be_mono)
{
    ttlet packed = make_packed();
    auto flat_samples = std::array<float, 256>{};

    ttlet unpacker = unpack_audio_samples{4, 0, 32, true, std::endian::big, 4};

    unpacker(packed.data(), flat_samples.data(), 1);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[0], packed[1], packed[2], packed[3]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 2);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[0], packed[1], packed[2], packed[3]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[4], packed[5], packed[6], packed[7]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 3);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[0], packed[1], packed[2], packed[3]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[4], packed[5], packed[6], packed[7]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[8], packed[9], packed[10], packed[11]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 4);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[0], packed[1], packed[2], packed[3]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[4], packed[5], packed[6], packed[7]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[8], packed[9], packed[10], packed[11]), 0.00001f);
    ASSERT_NEAR(flat_samples[3], float32_to_float(packed[12], packed[13], packed[14], packed[15]), 0.00001f);

    memset(flat_samples.data(), 0, sizeof(float) * 256);
    unpacker(packed.data(), flat_samples.data(), 8);
    ASSERT_NEAR(flat_samples[0], float32_to_float(packed[0], packed[1], packed[2], packed[3]), 0.00001f);
    ASSERT_NEAR(flat_samples[1], float32_to_float(packed[4], packed[5], packed[6], packed[7]), 0.00001f);
    ASSERT_NEAR(flat_samples[2], float32_to_float(packed[8], packed[9], packed[10], packed[11]), 0.00001f);
    ASSERT_NEAR(flat_samples[3], float32_to_float(packed[12], packed[13], packed[14], packed[15]), 0.00001f);
    ASSERT_NEAR(flat_samples[4], float32_to_float(packed[16], packed[17], packed[18], packed[19]), 0.00001f);
    ASSERT_NEAR(flat_samples[5], float32_to_float(packed[20], packed[21], packed[22], packed[23]), 0.00001f);
    ASSERT_NEAR(flat_samples[6], float32_to_float(packed[24], packed[25], packed[26], packed[27]), 0.00001f);
    ASSERT_NEAR(flat_samples[7], float32_to_float(packed[28], packed[29], packed[30], packed[31]), 0.00001f);
}
