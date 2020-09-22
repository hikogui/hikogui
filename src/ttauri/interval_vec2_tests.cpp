// Copyright 2020 Pokitec
// All rights reserved.

#include "interval_vec2.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>

using namespace std;
using namespace tt;

TEST(IntervalVec2, Add)
{
    ASSERT_EQ(interval_vec2(1.0f, 2.0f) + interval_vec2(3.0f, 4.0f), interval_vec2(4.0f, 6.0f));

    ASSERT_EQ(
        interval_vec2(vec(1.0f, 2.0f), vec(2.0f, 3.0f)) + interval_vec2(vec(3.0f, 4.0f), vec(4.0f, 5.0f)),
        interval_vec2(vec(4.0f, 6.0f), vec(6.0f, 8.0f)));
}

TEST(IntervalVec2, Max)
{
    ASSERT_EQ(
        max(interval_vec2(vec(136.0f, 56.0f), vec(136.0f, 59.0f)), interval_vec2(vec(150.0f, 0.0f), vec(150.0f, 0.0f))),
        interval_vec2(vec(150.0f, 56.0f), vec(150.0f, 59.0f)));

    ASSERT_EQ(
        max(interval_vec2(vec(1.0f, 2.0f), vec(2.0f, 3.0f)), interval_vec2(vec(3.0f, 4.0f), vec(4.0f, 5.0f))),
        interval_vec2(vec(3.0f, 4.0f), vec(4.0f, 5.0f)));
}
