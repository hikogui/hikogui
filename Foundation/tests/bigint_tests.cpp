// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/bigint.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <array>

using namespace std;
using namespace TTauri;

TEST(BigInt, Default) {
    {
        auto t = uint128_t{1};
        auto u = uint128_t{1};
        ASSERT_EQ(t, u);
    }

    {
        auto t = uint128_t{"1"};
        auto u = uint128_t{"1"};
        ASSERT_EQ(t, u);
    }

    {
        auto t = uint128_t{"123456789012345678901234567890"};
        auto u = uint128_t{"123456789012345678901234567890"};
        ASSERT_EQ(t, u);
    }

    {
        auto t = uint128_t{"25"};
        auto u = uint128_t{5};
        u *= 5;
        ASSERT_EQ(t, u);
    }

    {
        auto t = uint128_t{"9223372036854775857"};
        auto u = uint128_t{9223372036854775807};
        u+= 50;
        ASSERT_EQ(t, u);
    }

}