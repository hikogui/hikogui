// Copyright 2021 Pokitec
// All rights reserved.

#include "ttauri/geometry/vec.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(vec, compare)
{
    ASSERT_FALSE(vec2(1.0, 2.0) == vec2(3.0, 4.0));
    ASSERT_FALSE(vec2(1.0, 2.0) == vec2(1.0, 4.0));
    ASSERT_FALSE(vec2(1.0, 2.0) == vec2(3.0, 2.0));
    ASSERT_TRUE(vec2(1.0, 2.0) == vec2(1.0, 2.0));

    ASSERT_TRUE(vec2(1.0, 2.0) != vec2(3.0, 4.0));
    ASSERT_TRUE(vec2(1.0, 2.0) != vec2(1.0, 4.0));
    ASSERT_TRUE(vec2(1.0, 2.0) != vec2(3.0, 2.0));
    ASSERT_FALSE(vec2(1.0, 2.0) != vec2(1.0, 2.0));

    ASSERT_FALSE(vec3(1.0, 2.0, 3.0) == vec3(3.0, 4.0, 5.0));
    ASSERT_FALSE(vec3(1.0, 2.0, 3.0) == vec3(1.0, 4.0, 5.0));
    ASSERT_FALSE(vec3(1.0, 2.0, 3.0) == vec3(3.0, 2.0, 5.0));
    ASSERT_TRUE(vec3(1.0, 2.0, 3.0) == vec3(1.0, 2.0, 3.0));

    ASSERT_FALSE(vec2(1.0, 2.0) == vec3(3.0, 4.0, 5.0));
    ASSERT_FALSE(vec2(1.0, 2.0) == vec3(1.0, 4.0, 5.0));
    ASSERT_FALSE(vec2(1.0, 2.0) == vec3(3.0, 2.0, 5.0));
    ASSERT_FALSE(vec2(1.0, 2.0) == vec3(1.0, 2.0, 3.0));
    ASSERT_TRUE(vec2(1.0, 2.0) == vec3(1.0, 2.0, 0.0));
}

TEST(vec, adding)
{
    ASSERT_EQ(vec2(1.0, 2.0) + vec2(3.0, 4.0), vec2(4.0, 6.0));
    ASSERT_EQ(vec3(1.0, 2.0, 3.0) + vec3(3.0, 4.0, 5.0), vec3(4.0, 6.0, 8.0));
    ASSERT_EQ(vec2(1.0, 2.0) + vec3(3.0, 4.0, 5.0), vec3(4.0, 6.0, 5.0));
    ASSERT_EQ(vec3(1.0, 2.0, 3.0) + vec2(3.0, 4.0), vec3(4.0, 6.0, 3.0));

    static_assert(vec2(1.0, 2.0) + vec2(3.0, 4.0) == vec2(4.0, 6.0));
    static_assert(vec3(1.0, 2.0, 3.0) + vec3(3.0, 4.0, 5.0) == vec3(4.0, 6.0, 8.0));
    static_assert(vec2(1.0, 2.0) + vec3(3.0, 4.0, 5.0) == vec3(4.0, 6.0, 5.0));
    static_assert(vec3(1.0, 2.0, 3.0) + vec2(3.0, 4.0) == vec3(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(vec2(1.0, 2.0) + vec2(3.0, 4.0)), vec2>);
    static_assert(std::is_same_v<decltype(vec3(1.0, 2.0, 3.0) + vec2(3.0, 4.0)), vec3>);
    static_assert(std::is_same_v<decltype(vec2(1.0, 2.0) + vec3(3.0, 4.0, 5.0)), vec3>);
    static_assert(std::is_same_v<decltype(vec3(1.0, 2.0, 3.0) + vec3(3.0, 4.0, 5.0)), vec3>);
}

TEST(vec, subtracting)
{
    ASSERT_EQ(vec2(1.0, 2.0) - vec2(3.0, 4.0), vec2(-2.0, -2.0));
    ASSERT_EQ(vec3(1.0, 2.0, 3.0) - vec3(3.0, 4.0, 5.0), vec3(-2.0, -2.0, -2.0));
    ASSERT_EQ(vec2(1.0, 2.0) - vec3(3.0, 4.0, 5.0), vec3(-2.0, -2.0, -5.0));
    ASSERT_EQ(vec3(1.0, 2.0, 3.0) - vec2(3.0, 4.0), vec3(-2.0, -2.0, 3.0));

    static_assert(vec2(1.0, 2.0) - vec2(3.0, 4.0) == vec2(-2.0, -2.0));
    static_assert(vec3(1.0, 2.0, 3.0) - vec3(3.0, 4.0, 5.0) == vec3(-2.0, -2.0, -2.0));
    static_assert(vec2(1.0, 2.0) - vec3(3.0, 4.0, 5.0) == vec3(-2.0, -2.0, -5.0));
    static_assert(vec3(1.0, 2.0, 3.0) - vec2(3.0, 4.0) == vec3(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(vec2(1.0, 2.0) - vec2(3.0, 4.0)), vec2>);
    static_assert(std::is_same_v<decltype(vec3(1.0, 2.0, 3.0) - vec2(3.0, 4.0)), vec3>);
    static_assert(std::is_same_v<decltype(vec2(1.0, 2.0) - vec3(3.0, 4.0, 5.0)), vec3>);
    static_assert(std::is_same_v<decltype(vec3(1.0, 2.0, 3.0) - vec3(3.0, 4.0, 5.0)), vec3>);
}

TEST(vec, scaling)
{
    ASSERT_EQ(vec2(1.0, 2.0) * 42.0, vec2(42.0, 84.0));
    ASSERT_EQ(vec3(1.0, 2.0, 3.0) * 42.0, vec3(42.0, 84.0, 126.0));

    static_assert(vec2(1.0, 2.0) * 42.0 == vec2(42.0, 84.0));
    static_assert(vec3(1.0, 2.0, 3.0) * 42.0 == vec3(42.0, 84.0, 126.0));

    static_assert(std::is_same_v<decltype(vec3(1.0, 2.0, 3.0) * 42.0), vec3>);
    static_assert(std::is_same_v<decltype(vec2(1.0, 2.0) * 42.0), vec2>);
}

TEST(vec, invert)
{
    ASSERT_EQ(-vec2(1.0, 2.0), vec2(-1.0, -2.0));
    ASSERT_EQ(-vec3(1.0, 2.0, 3.0), vec3(-1.0, -2.0, -3.0));

    static_assert(-vec2(1.0, 2.0) == vec2(-1.0, -2.0));
    static_assert(-vec3(1.0, 2.0, 3.0) == vec3(-1.0, -2.0, -3.0));

    static_assert(std::is_same_v<decltype(-vec3(1.0, 2.0, 3.0)), vec3>);
    static_assert(std::is_same_v<decltype(-vec2(1.0, 2.0)), vec2>);
}

TEST(vec, hypot)
{
    ASSERT_NEAR(hypot(vec2(1.0, 2.0)), 2.236067, 0.00001);
    ASSERT_NEAR(hypot(vec3(1.0, 2.0, 3.0)), 3.741657, 0.00001);
}

TEST(vec, rcp_hypot)
{
    ASSERT_NEAR(rcp_hypot(vec2(1.0, 2.0)), 0.447213, 0.0001);
    ASSERT_NEAR(rcp_hypot(vec3(1.0, 2.0, 3.0)), 0.267261, 0.0001);
}

TEST(vec, rcp_normalize)
{
    ASSERT_NEAR(hypot(normalize(vec2(1.0, 2.0))), 1.0, 0.001);
    ASSERT_NEAR(hypot(normalize(vec3(1.0, 2.0, 3.0))), 1.0, 0.001);

    static_assert(std::is_same_v<decltype(normalize(vec3(1.0, 2.0, 3.0))), vec3>);
    static_assert(std::is_same_v<decltype(normalize(vec2(1.0, 2.0))), vec2>);
}

TEST(vec, dot)
{
    ASSERT_EQ(dot(vec2(1.0, 2.0), vec2(3.0, 4.0)), 11.0);
    ASSERT_EQ(dot(vec2(1.0, 2.0), vec3(3.0, 4.0, 5.0)), 11.0);
    ASSERT_EQ(dot(vec3(1.0, 2.0, 3.0), vec2(3.0, 4.0)), 11.0);
    ASSERT_EQ(dot(vec3(1.0, 2.0, 3.0), vec3(3.0, 4.0, 5.0)), 26.0);

    static_assert(dot(vec2(1.0, 2.0), vec2(3.0, 4.0)) == 11.0);
    static_assert(dot(vec2(1.0, 2.0), vec3(3.0, 4.0, 5.0)) == 11.0);
    static_assert(dot(vec3(1.0, 2.0, 3.0), vec2(3.0, 4.0)) == 11.0);
    static_assert(dot(vec3(1.0, 2.0, 3.0), vec3(3.0, 4.0, 5.0)) == 26.0);
}

TEST(vec, cross)
{
    ASSERT_EQ(cross(vec2(4.0, 9.0)), vec2(-9.0, 4.0));
    ASSERT_EQ(cross(vec2(4.0, 9.0), vec2(4.0, 9.0)), 0.0);
    ASSERT_EQ(cross(vec2(4.0, 9.0), vec2(-9.0, 4.0)), 97.0);
    ASSERT_EQ(cross(vec3(3.0, -3.0, 1.0), vec3(4.0, 9.0, 2.0)), vec3(-15.0, -2.0, 39.0));

    static_assert(cross(vec2(4.0, 9.0)) == vec2(-9.0, 4.0));
    static_assert(cross(vec2(4.0, 9.0), vec2(4.0, 9.0)) == 0.0);
    static_assert(cross(vec2(4.0, 9.0), vec2(-9.0, 4.0)) == 97.0);
    static_assert(cross(vec3(3.0, -3.0, 1.0), vec3(4.0, 9.0, 2.0)) == vec3(-15.0, -2.0, 39.0));
}
