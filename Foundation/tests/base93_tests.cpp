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

    for (int64_t count = 0; count < 1000; count++) {
        for (int repeat = 0; repeat < 10; repeat++) {

            let message = random_generator.get_bytes(count);

            let text = base93_encode(message);

            let result = base93_decode(text);

            ASSERT_EQ(message, result);
        }
    }

}