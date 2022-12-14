// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "seed.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>

TEST(seed, entropy)
{
    auto count = std::array<int,16>{};

    for (auto i = 0; i != 100; ++i) {
        // Bigger than a byte, tests the second byte.
        auto value = hi::seed<uint16_t>{}();

        // Count how often each bit is set.
        for (auto j = 0; j != 16; ++j) {
            if ((value >> j) & 1) {
                ++count[j];
            }
        }
    }

    for (auto j = 0; j != 16; ++j) {
        ASSERT_TRUE(count[j] >= 25 and count[j] <= 75) << std::format("Bit {} was set {}/100",j , count[j]);
    }
}
