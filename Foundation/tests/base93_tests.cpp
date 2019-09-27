// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/base93.hpp"
#include "TTauri/Foundation/random_pcg.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace std;
using namespace TTauri;

TEST(Base93, Default) {
    pcg32 random_generator;

    std::vector<ssize_t> counts;

    for (ssize_t i = 0; i < 28; i++) {
        counts.push_back(i);
    }

#if defined(NDEBUG)
    constexpr ssize_t nr_random_sizes = 20000;
#else
    constexpr ssize_t nr_random_sizes = 20;
#endif
    for (ssize_t i = 0; i < nr_random_sizes; i++) {
        counts.push_back(random_generator() % 2000);
    }

    for (let count: counts) {
        let message = random_generator.get_bytes(count);

        let text = base93_encode(message);

        let result = base93_decode(text);

        ASSERT_EQ(message, result);
    }

}