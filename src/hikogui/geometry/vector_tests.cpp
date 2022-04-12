// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/geometry/vector.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi::geo;
TEST(vec, compare)
{
    ASSERT_FALSE(vector<2>(1.0, 2.0) == vector<2>(3.0, 4.0));
    ASSERT_FALSE(vector<2>(1.0, 2.0) == vector<2>(1.0, 4.0));
    ASSERT_FALSE(vector<2>(1.0, 2.0) == vector<2>(3.0, 2.0));
    ASSERT_TRUE(vector<2>(1.0, 2.0) == vector<2>(1.0, 2.0));

    ASSERT_TRUE(vector<2>(1.0, 2.0) != vector<2>(3.0, 4.0));
    ASSERT_TRUE(vector<2>(1.0, 2.0) != vector<2>(1.0, 4.0));
    ASSERT_TRUE(vector<2>(1.0, 2.0) != vector<2>(3.0, 2.0));
    ASSERT_FALSE(vector<2>(1.0, 2.0) != vector<2>(1.0, 2.0));

    ASSERT_FALSE(vector<3>(1.0, 2.0, 3.0) == vector<3>(3.0, 4.0, 5.0));
    ASSERT_FALSE(vector<3>(1.0, 2.0, 3.0) == vector<3>(1.0, 4.0, 5.0));
    ASSERT_FALSE(vector<3>(1.0, 2.0, 3.0) == vector<3>(3.0, 2.0, 5.0));
    ASSERT_TRUE(vector<3>(1.0, 2.0, 3.0) == vector<3>(1.0, 2.0, 3.0));

    ASSERT_FALSE(vector<2>(1.0, 2.0) == vector<3>(3.0, 4.0, 5.0));
    ASSERT_FALSE(vector<2>(1.0, 2.0) == vector<3>(1.0, 4.0, 5.0));
    ASSERT_FALSE(vector<2>(1.0, 2.0) == vector<3>(3.0, 2.0, 5.0));
    ASSERT_FALSE(vector<2>(1.0, 2.0) == vector<3>(1.0, 2.0, 3.0));
    ASSERT_TRUE(vector<2>(1.0, 2.0) == vector<3>(1.0, 2.0, 0.0));
}

TEST(vec, adding)
{
    ASSERT_EQ(vector<2>(1.0, 2.0) + vector<2>(3.0, 4.0), vector<2>(4.0, 6.0));
    ASSERT_EQ(vector<3>(1.0, 2.0, 3.0) + vector<3>(3.0, 4.0, 5.0), vector<3>(4.0, 6.0, 8.0));
    ASSERT_EQ(vector<2>(1.0, 2.0) + vector<3>(3.0, 4.0, 5.0), vector<3>(4.0, 6.0, 5.0));
    ASSERT_EQ(vector<3>(1.0, 2.0, 3.0) + vector<2>(3.0, 4.0), vector<3>(4.0, 6.0, 3.0));

    static_assert(vector<2>(1.0, 2.0) + vector<2>(3.0, 4.0) == vector<2>(4.0, 6.0));
    static_assert(vector<3>(1.0, 2.0, 3.0) + vector<3>(3.0, 4.0, 5.0) == vector<3>(4.0, 6.0, 8.0));
    static_assert(vector<2>(1.0, 2.0) + vector<3>(3.0, 4.0, 5.0) == vector<3>(4.0, 6.0, 5.0));
    static_assert(vector<3>(1.0, 2.0, 3.0) + vector<2>(3.0, 4.0) == vector<3>(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(vector<2>(1.0, 2.0) + vector<2>(3.0, 4.0)), vector<2>>);
    static_assert(std::is_same_v<decltype(vector<3>(1.0, 2.0, 3.0) + vector<2>(3.0, 4.0)), vector<3>>);
    static_assert(std::is_same_v<decltype(vector<2>(1.0, 2.0) + vector<3>(3.0, 4.0, 5.0)), vector<3>>);
    static_assert(std::is_same_v<decltype(vector<3>(1.0, 2.0, 3.0) + vector<3>(3.0, 4.0, 5.0)), vector<3>>);
}

TEST(vec, subtracting)
{
    ASSERT_EQ(vector<2>(1.0, 2.0) - vector<2>(3.0, 4.0), vector<2>(-2.0, -2.0));
    ASSERT_EQ(vector<3>(1.0, 2.0, 3.0) - vector<3>(3.0, 4.0, 5.0), vector<3>(-2.0, -2.0, -2.0));
    ASSERT_EQ(vector<2>(1.0, 2.0) - vector<3>(3.0, 4.0, 5.0), vector<3>(-2.0, -2.0, -5.0));
    ASSERT_EQ(vector<3>(1.0, 2.0, 3.0) - vector<2>(3.0, 4.0), vector<3>(-2.0, -2.0, 3.0));

    static_assert(vector<2>(1.0, 2.0) - vector<2>(3.0, 4.0) == vector<2>(-2.0, -2.0));
    static_assert(vector<3>(1.0, 2.0, 3.0) - vector<3>(3.0, 4.0, 5.0) == vector<3>(-2.0, -2.0, -2.0));
    static_assert(vector<2>(1.0, 2.0) - vector<3>(3.0, 4.0, 5.0) == vector<3>(-2.0, -2.0, -5.0));
    static_assert(vector<3>(1.0, 2.0, 3.0) - vector<2>(3.0, 4.0) == vector<3>(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(vector<2>(1.0, 2.0) - vector<2>(3.0, 4.0)), vector<2>>);
    static_assert(std::is_same_v<decltype(vector<3>(1.0, 2.0, 3.0) - vector<2>(3.0, 4.0)), vector<3>>);
    static_assert(std::is_same_v<decltype(vector<2>(1.0, 2.0) - vector<3>(3.0, 4.0, 5.0)), vector<3>>);
    static_assert(std::is_same_v<decltype(vector<3>(1.0, 2.0, 3.0) - vector<3>(3.0, 4.0, 5.0)), vector<3>>);
}

TEST(vec, scaling)
{
    ASSERT_EQ(vector<2>(1.0, 2.0) * 42.0, vector<2>(42.0, 84.0));
    ASSERT_EQ(vector<3>(1.0, 2.0, 3.0) * 42.0, vector<3>(42.0, 84.0, 126.0));

    static_assert(vector<2>(1.0, 2.0) * 42.0 == vector<2>(42.0, 84.0));
    static_assert(vector<3>(1.0, 2.0, 3.0) * 42.0 == vector<3>(42.0, 84.0, 126.0));

    static_assert(std::is_same_v<decltype(vector<3>(1.0, 2.0, 3.0) * 42.0), vector<3>>);
    static_assert(std::is_same_v<decltype(vector<2>(1.0, 2.0) * 42.0), vector<2>>);
}

TEST(vec, invert)
{
    ASSERT_EQ(-vector<2>(1.0, 2.0), vector<2>(-1.0, -2.0));
    ASSERT_EQ(-vector<3>(1.0, 2.0, 3.0), vector<3>(-1.0, -2.0, -3.0));

    static_assert(-vector<2>(1.0, 2.0) == vector<2>(-1.0, -2.0));
    static_assert(-vector<3>(1.0, 2.0, 3.0) == vector<3>(-1.0, -2.0, -3.0));

    static_assert(std::is_same_v<decltype(-vector<3>(1.0, 2.0, 3.0)), vector<3>>);
    static_assert(std::is_same_v<decltype(-vector<2>(1.0, 2.0)), vector<2>>);
}

TEST(vec, hypot)
{
    ASSERT_NEAR(hypot(vector<2>(1.0, 2.0)), 2.236067, 0.00001);
    ASSERT_NEAR(hypot(vector<3>(1.0, 2.0, 3.0)), 3.741657, 0.00001);
}

TEST(vec, rcp_hypot)
{
    ASSERT_NEAR(rcp_hypot(vector<2>(1.0, 2.0)), 0.447213, 0.0001);
    ASSERT_NEAR(rcp_hypot(vector<3>(1.0, 2.0, 3.0)), 0.267261, 0.0001);
}

TEST(vec, rcp_normalize)
{
    ASSERT_NEAR(hypot(normalize(vector<2>(1.0, 2.0))), 1.0, 0.001);
    ASSERT_NEAR(hypot(normalize(vector<3>(1.0, 2.0, 3.0))), 1.0, 0.001);

    static_assert(std::is_same_v<decltype(normalize(vector<3>(1.0, 2.0, 3.0))), vector<3>>);
    static_assert(std::is_same_v<decltype(normalize(vector<2>(1.0, 2.0))), vector<2>>);
}

TEST(vec, dot)
{
    ASSERT_EQ(dot(vector<2>(1.0, 2.0), vector<2>(3.0, 4.0)), 11.0);
    ASSERT_EQ(dot(vector<2>(1.0, 2.0), vector<3>(3.0, 4.0, 5.0)), 11.0);
    ASSERT_EQ(dot(vector<3>(1.0, 2.0, 3.0), vector<2>(3.0, 4.0)), 11.0);
    ASSERT_EQ(dot(vector<3>(1.0, 2.0, 3.0), vector<3>(3.0, 4.0, 5.0)), 26.0);

    static_assert(dot(vector<2>(1.0, 2.0), vector<2>(3.0, 4.0)) == 11.0);
    static_assert(dot(vector<2>(1.0, 2.0), vector<3>(3.0, 4.0, 5.0)) == 11.0);
    static_assert(dot(vector<3>(1.0, 2.0, 3.0), vector<2>(3.0, 4.0)) == 11.0);
    static_assert(dot(vector<3>(1.0, 2.0, 3.0), vector<3>(3.0, 4.0, 5.0)) == 26.0);
}

TEST(vec, cross)
{
    ASSERT_EQ(cross(vector<2>(4.0, 9.0)), vector<2>(-9.0, 4.0));
    ASSERT_EQ(cross(vector<2>(4.0, 9.0), vector<2>(4.0, 9.0)), 0.0);
    ASSERT_EQ(cross(vector<2>(4.0, 9.0), vector<2>(-9.0, 4.0)), 97.0);
    ASSERT_EQ(cross(vector<3>(3.0, -3.0, 1.0), vector<3>(4.0, 9.0, 2.0)), vector<3>(-15.0, -2.0, 39.0));

    static_assert(cross(vector<2>(4.0, 9.0)) == vector<2>(-9.0, 4.0));
    static_assert(cross(vector<2>(4.0, 9.0), vector<2>(4.0, 9.0)) == 0.0);
    static_assert(cross(vector<2>(4.0, 9.0), vector<2>(-9.0, 4.0)) == 97.0);
    static_assert(cross(vector<3>(3.0, -3.0, 1.0), vector<3>(4.0, 9.0, 2.0)) == vector<3>(-15.0, -2.0, 39.0));
}
