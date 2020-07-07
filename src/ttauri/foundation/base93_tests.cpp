// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/base93.hpp"
#include "ttauri/foundation/random_pcg.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace std;
using namespace tt;

TEST(Base93, Default) {
    pcg32 random_generator;

    std::vector<ssize_t> counts;

    for (ssize_t i = 0; i < 28; i++) {
        counts.push_back(i);
    }

    constexpr ssize_t nr_random_sizes = BuildType::current == BuildType::Release ? 20000 : 20;

    for (ssize_t i = 0; i < nr_random_sizes; i++) {
        counts.push_back(random_generator() % 2000);
    }

    for (ttlet count: counts) {
        ttlet message = random_generator.get_bytes(count);

        ttlet text = base93_encode(message);

        ttlet result = base93_decode(text);

        ASSERT_EQ(message, result);
    }

}