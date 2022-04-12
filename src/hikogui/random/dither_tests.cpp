// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "dither.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>
#include <map>
#include <cmath>

using namespace std;
using namespace hi;

[[nodiscard]] static std::map<int, float> dither_test(int num_bits, float sample_value) noexcept
{
    auto sample_count = 10000;
    auto sample_percentage = 100.0f / (sample_count * 4);

    // Signed 16 bit PCM has 15 bits of fraction.
    auto d = dither(num_bits);

    // The maximum value of a 16 bit PCM sample.
    auto max_sample_value = static_cast<float>((1_uz << num_bits) - 1);
    auto rcp_max_sample_value = 1.0f / max_sample_value;
    auto scaled_sample_value = f32x4::broadcast(sample_value * rcp_max_sample_value);

    auto results = std::map<int, float>{};
    for (auto i = 0; i != sample_count; ++i) {
        auto dither_value = d.next();
        auto dithered_sample_value = scaled_sample_value + dither_value;

        auto result_sample_values = i32x4{dithered_sample_value * max_sample_value};

        for (int j = 0; j != result_sample_values.size(); ++j) {
            auto result_sample_value = result_sample_values[j];
            auto it = results.find(result_sample_value);
            if (it == results.end()) {
                results[result_sample_value] = sample_percentage;
            } else {
                it->second += sample_percentage;
            }
        }
    }

    return results;
}

TEST(dither, PCM8_1_0)
{
    auto results = dither_test(7, 1.0f);

    EXPECT_NEAR(results[-1], 0.0f, 0.1f);
    EXPECT_NEAR(results[0], 12.5f, 5.0f);
    EXPECT_NEAR(results[1], 75.0f, 5.0f);
    EXPECT_NEAR(results[2], 12.5f, 5.0f);
    EXPECT_NEAR(results[3], 0.0f, 0.1f);
}

TEST(dither, PCM16_1_0)
{
    auto results = dither_test(15, 1.0f);

    EXPECT_NEAR(results[-1], 0.0f, 0.1f);
    EXPECT_NEAR(results[0], 12.5f, 5.0f);
    EXPECT_NEAR(results[1], 75.0f, 5.0f);
    EXPECT_NEAR(results[2], 12.5f, 5.0f);
    EXPECT_NEAR(results[3], 0.0f, 0.1f);
}

TEST(dither, PCM24_1_0)
{
    auto results = dither_test(23, 1.0f);

    EXPECT_NEAR(results[-1], 0.0f, 0.1f);
    EXPECT_NEAR(results[0], 12.5f, 5.0f);
    EXPECT_NEAR(results[1], 75.0f, 5.0f);
    EXPECT_NEAR(results[2], 12.5f, 5.0f);
    EXPECT_NEAR(results[3], 0.0f, 0.1f);
}

TEST(dither, PCM8_1_5)
{
    auto results = dither_test(7, 1.5f);

    EXPECT_NEAR(results[0], 0.0f, 0.1f);
    EXPECT_NEAR(results[1], 50.0f, 5.0f);
    EXPECT_NEAR(results[2], 50.0f, 5.0f);
    EXPECT_NEAR(results[3], 0.0f, 0.1f);
}

TEST(dither, PCM16_1_5)
{
    auto results = dither_test(15, 1.5f);

    EXPECT_NEAR(results[0], 0.0f, 0.1f);
    EXPECT_NEAR(results[1], 50.0f, 5.0f);
    EXPECT_NEAR(results[2], 50.0f, 5.0f);
    EXPECT_NEAR(results[3], 0.0f, 0.1f);
}

TEST(dither, PCM24_1_5)
{
    auto results = dither_test(23, 1.5f);

    EXPECT_NEAR(results[0], 0.0f, 0.1f);
    EXPECT_NEAR(results[1], 50.0f, 5.0f);
    EXPECT_NEAR(results[2], 50.0f, 5.0f);
    EXPECT_NEAR(results[3], 0.0f, 0.1f);
}
