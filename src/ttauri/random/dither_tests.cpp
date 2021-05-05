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
using namespace tt;

[[nodiscard]] static std::map<int,float> dither_test(int num_bits, float sample_value) noexcept
{
    auto sample_count = 10000;
    auto sample_percentage = 100.0f / (sample_count * 8);

    // Signed 16 bit PCM has 15 bits of fraction.
    auto d = dither(num_bits);

    // The maximum value of a 16 bit PCM sample.
    auto max_sample_value = static_cast<float>((1_uz << num_bits) - 1);
    auto rcp_max_sample_value = 1.0f / max_sample_value;
    auto scaled_sample_value = f32x8::broadcast(sample_value * rcp_max_sample_value);

    auto results = std::map<int, float>{};
    for (auto i = 0; i != sample_count; ++i) {
        auto dithered_sample_value = d.next(scaled_sample_value);

        auto result_sample_value = i32x8{dithered_sample_value * max_sample_value};

        for (int j = 0; j != result_sample_value.size(); ++j) {
            auto it = results.find(result_sample_value[j]);
            if (it == results.end()) {
                results[result_sample_value[j]] = sample_percentage;
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

    ASSERT_TRUE(results[0] > 11.5f and results[0] < 13.5f);
    ASSERT_TRUE(results[1] > 74.0f and results[1] < 76.0f);
    ASSERT_TRUE(results[2] > 11.5f and results[2] < 13.5f);
}

TEST(dither, PCM16_1_0)
{
    auto results = dither_test(15, 1.0f);

    ASSERT_TRUE(results[0] > 11.5f and results[0] < 13.5f);
    ASSERT_TRUE(results[1] > 74.0f and results[1] < 76.0f);
    ASSERT_TRUE(results[2] > 11.5f and results[2] < 13.5f);
}

TEST(dither, PCM24_1_0)
{
    auto results = dither_test(23, 1.0f);

    ASSERT_TRUE(results[0] > 11.5f and results[0] < 13.5f);
    ASSERT_TRUE(results[1] > 74.0f and results[1] < 76.0f);
    ASSERT_TRUE(results[2] > 11.5f and results[2] < 13.5f);
}

TEST(dither, PCM8_1_5)
{
    auto results = dither_test(7, 1.5f);

    ASSERT_TRUE(results[1] > 49.0f and results[1] < 51.0f);
    ASSERT_TRUE(results[2] > 49.0f and results[2] < 51.0f);
}

TEST(dither, PCM16_1_5)
{
    auto results = dither_test(15, 1.5f);

    ASSERT_TRUE(results[1] > 49.0f and results[1] < 51.0f);
    ASSERT_TRUE(results[2] > 49.0f and results[2] < 51.0f);
}

TEST(dither, PCM24_1_5)
{
    auto results = dither_test(23, 1.5f);

    ASSERT_TRUE(results[1] > 49.0f and results[1] < 51.0f);
    ASSERT_TRUE(results[2] > 49.0f and results[2] < 51.0f);
}
