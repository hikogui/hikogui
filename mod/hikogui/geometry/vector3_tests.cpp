// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "vector3.hpp"
#include "../utility/utility.hpp"
#include "../test.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>



using namespace hi;

TEST(vector3, compare)
{
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

TEST(vector3, adding)
{
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) + vector3(3.0, 4.0, 5.0) == vector3(4.0, 6.0, 8.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) + vector3(3.0, 4.0, 5.0) == vector3(4.0, 6.0, 5.0));
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) + vector2(3.0, 4.0) == vector3(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) + vector2(3.0, 4.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + vector3(3.0, 4.0, 5.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) + vector3(3.0, 4.0, 5.0)), vector3>);
}

TEST(vector3, subtracting)
{
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) - vector3(3.0, 4.0, 5.0) == vector3(-2.0, -2.0, -2.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) - vector3(3.0, 4.0, 5.0) == vector3(-2.0, -2.0, -5.0));
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) - vector2(3.0, 4.0) == vector3(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) - vector2(3.0, 4.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) - vector3(3.0, 4.0, 5.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) - vector3(3.0, 4.0, 5.0)), vector3>);
}

TEST(vector3, scaling)
{
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) * 42.0 == vector3(42.0, 84.0, 126.0));

    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) * 42.0), vector3>);
}

TEST(vector3, invert)
{
    STATIC_ASSERT_TRUE(-vector3(1.0, 2.0, 3.0) == vector3(-1.0, -2.0, -3.0));

    static_assert(std::is_same_v<decltype(-vector3(1.0, 2.0, 3.0)), vector3>);
}

TEST(vector3, hypot)
{
    ASSERT_NEAR(hypot(vector3(1.0, 2.0, 3.0)), 3.741657, 0.00001);
}

TEST(vector3, rcp_hypot)
{
    ASSERT_NEAR(rcp_hypot(vector3(1.0, 2.0, 3.0)), 0.267261, 0.0001);
}

TEST(vector3, rcp_normalize)
{
    ASSERT_NEAR(hypot(normalize(vector3(1.0, 2.0, 3.0))), 1.0, 0.001);

    static_assert(std::is_same_v<decltype(normalize(vector3(1.0, 2.0, 3.0))), vector3>);
}

TEST(vector3, dot)
{
    STATIC_ASSERT_TRUE(dot(vector2(1.0, 2.0), vector3(3.0, 4.0, 5.0)) == 11.0);
    STATIC_ASSERT_TRUE(dot(vector3(1.0, 2.0, 3.0), vector2(3.0, 4.0)) == 11.0);
    STATIC_ASSERT_TRUE(dot(vector3(1.0, 2.0, 3.0), vector3(3.0, 4.0, 5.0)) == 26.0);
}

TEST(vector3, cross)
{
    STATIC_ASSERT_TRUE(cross(vector3(3.0, -3.0, 1.0), vector3(4.0, 9.0, 2.0)) == vector3(-15.0, -2.0, 39.0));
}
