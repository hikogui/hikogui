// Copyright 2021 Pokitec
// All rights reserved.

#include "ttauri/geometry/point.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(point, compare)
{
    ASSERT_FALSE(point2(1.0, 2.0) == point2(3.0, 4.0));
    ASSERT_FALSE(point2(1.0, 2.0) == point2(1.0, 4.0));
    ASSERT_FALSE(point2(1.0, 2.0) == point2(3.0, 2.0));
    ASSERT_TRUE(point2(1.0, 2.0) == point2(1.0, 2.0));

    ASSERT_TRUE(point2(1.0, 2.0) != point2(3.0, 4.0));
    ASSERT_TRUE(point2(1.0, 2.0) != point2(1.0, 4.0));
    ASSERT_TRUE(point2(1.0, 2.0) != point2(3.0, 2.0));
    ASSERT_FALSE(point2(1.0, 2.0) != point2(1.0, 2.0));

    ASSERT_FALSE(point3(1.0, 2.0, 3.0) == point3(3.0, 4.0, 5.0));
    ASSERT_FALSE(point3(1.0, 2.0, 3.0) == point3(1.0, 4.0, 5.0));
    ASSERT_FALSE(point3(1.0, 2.0, 3.0) == point3(3.0, 2.0, 5.0));
    ASSERT_TRUE(point3(1.0, 2.0, 3.0) == point3(1.0, 2.0, 3.0));

    ASSERT_FALSE(point2(1.0, 2.0) == point3(3.0, 4.0, 5.0));
    ASSERT_FALSE(point2(1.0, 2.0) == point3(1.0, 4.0, 5.0));
    ASSERT_FALSE(point2(1.0, 2.0) == point3(3.0, 2.0, 5.0));
    ASSERT_FALSE(point2(1.0, 2.0) == point3(1.0, 2.0, 3.0));
    ASSERT_TRUE(point2(1.0, 2.0) == point3(1.0, 2.0, 0.0));
}

TEST(point, adding)
{
    ASSERT_EQ(point2(1.0, 2.0) + vec2(3.0, 4.0), point2(4.0, 6.0));
    ASSERT_EQ(point3(1.0, 2.0, 3.0) + vec3(3.0, 4.0, 5.0), point3(4.0, 6.0, 8.0));
    ASSERT_EQ(point2(1.0, 2.0) + vec3(3.0, 4.0, 5.0), point3(4.0, 6.0, 5.0));
    ASSERT_EQ(point3(1.0, 2.0, 3.0) + vec2(3.0, 4.0), point3(4.0, 6.0, 3.0));

    static_assert(point2(1.0, 2.0) + vec2(3.0, 4.0) == point2(4.0, 6.0));
    static_assert(point3(1.0, 2.0, 3.0) + vec3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 8.0));
    static_assert(point2(1.0, 2.0) + vec3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 5.0));
    static_assert(point3(1.0, 2.0, 3.0) + vec2(3.0, 4.0) == point3(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) + vec2(3.0, 4.0)), point2>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) + vec2(3.0, 4.0)), point3>);
    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) + vec3(3.0, 4.0, 5.0)), point3>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) + vec3(3.0, 4.0, 5.0)), point3>);

    ASSERT_EQ(vec2(1.0, 2.0) + point2(3.0, 4.0), point2(4.0, 6.0));
    ASSERT_EQ(vec3(1.0, 2.0, 3.0) + point3(3.0, 4.0, 5.0), point3(4.0, 6.0, 8.0));
    ASSERT_EQ(vec2(1.0, 2.0) + point3(3.0, 4.0, 5.0), point3(4.0, 6.0, 5.0));
    ASSERT_EQ(vec3(1.0, 2.0, 3.0) + point2(3.0, 4.0), point3(4.0, 6.0, 3.0));

    static_assert(vec2(1.0, 2.0) + point2(3.0, 4.0) == point2(4.0, 6.0));
    static_assert(vec3(1.0, 2.0, 3.0) + point3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 8.0));
    static_assert(vec2(1.0, 2.0) + point3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 5.0));
    static_assert(vec3(1.0, 2.0, 3.0) + point2(3.0, 4.0) == point3(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(vec2(1.0, 2.0) + point2(3.0, 4.0)), point2>);
    static_assert(std::is_same_v<decltype(vec3(1.0, 2.0, 3.0) + point2(3.0, 4.0)), point3>);
    static_assert(std::is_same_v<decltype(vec2(1.0, 2.0) + point3(3.0, 4.0, 5.0)), point3>);
    static_assert(std::is_same_v<decltype(vec3(1.0, 2.0, 3.0) + point3(3.0, 4.0, 5.0)), point3>);
}

TEST(point, subtracting)
{
    ASSERT_EQ(point2(1.0, 2.0) - point2(3.0, 4.0), vec2(-2.0, -2.0));
    ASSERT_EQ(point3(1.0, 2.0, 3.0) - point3(3.0, 4.0, 5.0), vec3(-2.0, -2.0, -2.0));
    ASSERT_EQ(point2(1.0, 2.0) - point3(3.0, 4.0, 5.0), vec3(-2.0, -2.0, -5.0));
    ASSERT_EQ(point3(1.0, 2.0, 3.0) - point2(3.0, 4.0), vec3(-2.0, -2.0, 3.0));

    static_assert(point2(1.0, 2.0) - point2(3.0, 4.0) == vec2(-2.0, -2.0));
    static_assert(point3(1.0, 2.0, 3.0) - point3(3.0, 4.0, 5.0) == vec3(-2.0, -2.0, -2.0));
    static_assert(point2(1.0, 2.0) - point3(3.0, 4.0, 5.0) == vec3(-2.0, -2.0, -5.0));
    static_assert(point3(1.0, 2.0, 3.0) - point2(3.0, 4.0) == vec3(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - point2(3.0, 4.0)), vec2>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - point2(3.0, 4.0)), vec3>);
    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - point3(3.0, 4.0, 5.0)), vec3>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - point3(3.0, 4.0, 5.0)), vec3>);

    ASSERT_EQ(point2(1.0, 2.0) - vec2(3.0, 4.0), point2(-2.0, -2.0));
    ASSERT_EQ(point3(1.0, 2.0, 3.0) - vec3(3.0, 4.0, 5.0), point3(-2.0, -2.0, -2.0));
    ASSERT_EQ(point2(1.0, 2.0) - vec3(3.0, 4.0, 5.0), point3(-2.0, -2.0, -5.0));
    ASSERT_EQ(point3(1.0, 2.0, 3.0) - vec2(3.0, 4.0), point3(-2.0, -2.0, 3.0));

    static_assert(point2(1.0, 2.0) - vec2(3.0, 4.0) == point2(-2.0, -2.0));
    static_assert(point3(1.0, 2.0, 3.0) - vec3(3.0, 4.0, 5.0) == point3(-2.0, -2.0, -2.0));
    static_assert(point2(1.0, 2.0) - vec3(3.0, 4.0, 5.0) == point3(-2.0, -2.0, -5.0));
    static_assert(point3(1.0, 2.0, 3.0) - vec2(3.0, 4.0) == point3(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - vec2(3.0, 4.0)), point2>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - vec2(3.0, 4.0)), point3>);
    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - vec3(3.0, 4.0, 5.0)), point3>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - vec3(3.0, 4.0, 5.0)), point3>);
}
