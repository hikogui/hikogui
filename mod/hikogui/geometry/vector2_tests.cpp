// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "vector2.hpp"
#include "../utility/utility.hpp"
#include "../test.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>



using namespace hi;

TEST(vector2, compare)
{
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) == vector2(3.0, 4.0));
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) == vector2(1.0, 4.0));
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) == vector2(3.0, 2.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) == vector2(1.0, 2.0));

    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) != vector2(3.0, 4.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) != vector2(1.0, 4.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) != vector2(3.0, 2.0));
    STATIC_ASSERT_FALSE(vector2(1.0, 2.0) != vector2(1.0, 2.0));
}

TEST(vector2, adding)
{
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) + vector2(3.0, 4.0) == vector2(4.0, 6.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + vector2(3.0, 4.0)), vector2>);
}

TEST(vector2, subtracting)
{
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) - vector2(3.0, 4.0) == vector2(-2.0, -2.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) - vector2(3.0, 4.0)), vector2>);
}

TEST(vector2, scaling)
{
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) * 42.0 == vector2(42.0, 84.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) * 42.0), vector2>);
}

TEST(vector2, invert)
{
    STATIC_ASSERT_TRUE(-vector2(1.0, 2.0) == vector2(-1.0, -2.0));

    static_assert(std::is_same_v<decltype(-vector2(1.0, 2.0)), vector2>);
}

TEST(vector2, hypot)
{
    ASSERT_NEAR(hypot(vector2(1.0, 2.0)), 2.236067, 0.00001);
}

TEST(vector2, rcp_hypot)
{
    ASSERT_NEAR(rcp_hypot(vector2(1.0, 2.0)), 0.447213, 0.0001);
}

TEST(vector2, rcp_normalize)
{
    ASSERT_NEAR(hypot(normalize(vector2(1.0, 2.0))), 1.0, 0.001);

    static_assert(std::is_same_v<decltype(normalize(vector2(1.0, 2.0))), vector2>);
}

TEST(vector2, dot)
{
    STATIC_ASSERT_TRUE(dot(vector2(1.0, 2.0), vector2(3.0, 4.0)) == 11.0);
}

TEST(vector2, cross)
{
    STATIC_ASSERT_TRUE(cross(vector2(4.0, 9.0)) == vector2(-9.0, 4.0));
    STATIC_ASSERT_TRUE(cross(vector2(4.0, 9.0), vector2(4.0, 9.0)) == 0.0);
    STATIC_ASSERT_TRUE(cross(vector2(4.0, 9.0), vector2(-9.0, 4.0)) == 97.0);
}
