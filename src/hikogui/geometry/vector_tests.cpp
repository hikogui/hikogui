// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "vector.hpp"
#include "../utility/module.hpp"
#include "../utility/test.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi;
using namespace hi::geo;

TEST(vec, compare)
{
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) == vector2(3.0, 4.0));
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) == vector2(1.0, 4.0));
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) == vector2(3.0, 2.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) == vector2(1.0, 2.0));

    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) != vector2(3.0, 4.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) != vector2(1.0, 4.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) != vector2(3.0, 2.0));
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) != vector2(1.0, 2.0));

    STATIC_ASSERT_FALSE(vector3(1.0, 2.0, 3.0) == vector3(3.0, 4.0, 5.0));
    STATIC_ASSERT_FALSE(vector3(1.0, 2.0, 3.0) == vector3(1.0, 4.0, 5.0));
    STATIC_ASSERT_FALSE(vector3(1.0, 2.0, 3.0) == vector3(3.0, 2.0, 5.0));
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) == vector3(1.0, 2.0, 3.0));

    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) == vector3(3.0, 4.0, 5.0));
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) == vector3(1.0, 4.0, 5.0));
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) == vector3(3.0, 2.0, 5.0));
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) == vector3(1.0, 2.0, 3.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) == vector3(1.0, 2.0, 0.0));
}

TEST(vec, adding)
{
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) + vector2(3.0, 4.0) == vector2(4.0, 6.0));
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) + vector3(3.0, 4.0, 5.0) == vector3(4.0, 6.0, 8.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) + vector3(3.0, 4.0, 5.0) == vector3(4.0, 6.0, 5.0));
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) + vector2(3.0, 4.0) == vector3(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + vector2(3.0, 4.0)), vector2>);
    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) + vector2(3.0, 4.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + vector3(3.0, 4.0, 5.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) + vector3(3.0, 4.0, 5.0)), vector3>);
}

TEST(vec, subtracting)
{
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) - vector2(3.0, 4.0) == vector2(-2.0, -2.0));
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) - vector3(3.0, 4.0, 5.0) == vector3(-2.0, -2.0, -2.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) - vector3(3.0, 4.0, 5.0) == vector3(-2.0, -2.0, -5.0));
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) - vector2(3.0, 4.0) == vector3(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) - vector2(3.0, 4.0)), vector2>);
    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) - vector2(3.0, 4.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) - vector3(3.0, 4.0, 5.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) - vector3(3.0, 4.0, 5.0)), vector3>);
}

TEST(vec, scaling)
{
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) * 42.0 == vector2(42.0, 84.0));
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) * 42.0 == vector3(42.0, 84.0, 126.0));

    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) * 42.0), vector3>);
    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) * 42.0), vector2>);
}

TEST(vec, invert)
{
    STATIC_ASSERT_TRUE(-vector2(1.0, 2.0) == vector2(-1.0, -2.0));
    STATIC_ASSERT_TRUE(-vector3(1.0, 2.0, 3.0) == vector3(-1.0, -2.0, -3.0));

    static_assert(std::is_same_v<decltype(-vector3(1.0, 2.0, 3.0)), vector3>);
    static_assert(std::is_same_v<decltype(-vector2(1.0, 2.0)), vector2>);
}

TEST(vec, hypot)
{
    ASSERT_NEAR(hypot(vector2(1.0, 2.0)), 2.236067, 0.00001);
    ASSERT_NEAR(hypot(vector3(1.0, 2.0, 3.0)), 3.741657, 0.00001);
}

TEST(vec, rcp_hypot)
{
    ASSERT_NEAR(rcp_hypot(vector2(1.0, 2.0)), 0.447213, 0.0001);
    ASSERT_NEAR(rcp_hypot(vector3(1.0, 2.0, 3.0)), 0.267261, 0.0001);
}

TEST(vec, rcp_normalize)
{
    ASSERT_NEAR(hypot(normalize(vector2(1.0, 2.0))), 1.0, 0.001);
    ASSERT_NEAR(hypot(normalize(vector3(1.0, 2.0, 3.0))), 1.0, 0.001);

    static_assert(std::is_same_v<decltype(normalize(vector3(1.0, 2.0, 3.0))), vector3>);
    static_assert(std::is_same_v<decltype(normalize(vector2(1.0, 2.0))), vector2>);
}

TEST(vec, dot)
{
    STATIC_ASSERT_TRUE(dot(vector2(1.0, 2.0), vector2(3.0, 4.0)) == 11.0);
    STATIC_ASSERT_TRUE(dot(vector2(1.0, 2.0), vector3(3.0, 4.0, 5.0)) == 11.0);
    STATIC_ASSERT_TRUE(dot(vector3(1.0, 2.0, 3.0), vector2(3.0, 4.0)) == 11.0);
    STATIC_ASSERT_TRUE(dot(vector3(1.0, 2.0, 3.0), vector3(3.0, 4.0, 5.0)) == 26.0);
}

TEST(vec, cross)
{
    STATIC_ASSERT_TRUE(cross(vector2(4.0, 9.0)) == vector2(-9.0, 4.0));
    STATIC_ASSERT_TRUE(cross(vector2(4.0, 9.0), vector2(4.0, 9.0)) == 0.0);
    STATIC_ASSERT_TRUE(cross(vector2(4.0, 9.0), vector2(-9.0, 4.0)) == 97.0);
    STATIC_ASSERT_TRUE(cross(vector3(3.0, -3.0, 1.0), vector3(4.0, 9.0, 2.0)) == vector3(-15.0, -2.0, 39.0));
}
