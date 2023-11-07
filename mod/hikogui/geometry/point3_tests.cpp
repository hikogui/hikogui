// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "point3.hpp"
#include "point2.hpp"
#include "../utility/utility.hpp"
#include "../test.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>



using namespace hi;

TEST(point3, compare)
{
    STATIC_ASSERT_FALSE(point3(1.0, 2.0, 3.0) == point3(3.0, 4.0, 5.0));
    STATIC_ASSERT_FALSE(point3(1.0, 2.0, 3.0) == point3(1.0, 4.0, 5.0));
    STATIC_ASSERT_FALSE(point3(1.0, 2.0, 3.0) == point3(3.0, 2.0, 5.0));
    STATIC_ASSERT_TRUE(point3(1.0, 2.0, 3.0) == point3(1.0, 2.0, 3.0));

    STATIC_ASSERT_FALSE(point2(1.0, 2.0) == point3(3.0, 4.0, 5.0));
    STATIC_ASSERT_FALSE(point2(1.0, 2.0) == point3(1.0, 4.0, 5.0));
    STATIC_ASSERT_FALSE(point2(1.0, 2.0) == point3(3.0, 2.0, 5.0));
    STATIC_ASSERT_FALSE(point2(1.0, 2.0) == point3(1.0, 2.0, 3.0));
    STATIC_ASSERT_TRUE(point2(1.0, 2.0) == point3(1.0, 2.0, 0.0));
}

TEST(point3, adding)
{
    STATIC_ASSERT_TRUE(point3(1.0, 2.0, 3.0) + vector3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 8.0));
    STATIC_ASSERT_TRUE(point2(1.0, 2.0) + vector3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 5.0));
    STATIC_ASSERT_TRUE(point3(1.0, 2.0, 3.0) + vector2(3.0, 4.0) == point3(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) + vector2(3.0, 4.0)), point3>);
    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) + vector3(3.0, 4.0, 5.0)), point3>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) + vector3(3.0, 4.0, 5.0)), point3>);

    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) + point3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 8.0));
    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) + point3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 5.0));
    STATIC_ASSERT_TRUE(vector3(1.0, 2.0, 3.0) + point2(3.0, 4.0) == point3(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) + point2(3.0, 4.0)), point3>);
    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + point3(3.0, 4.0, 5.0)), point3>);
    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) + point3(3.0, 4.0, 5.0)), point3>);
}

TEST(point3, subtracting)
{
    STATIC_ASSERT_TRUE(point3(1.0, 2.0, 3.0) - point3(3.0, 4.0, 5.0) == vector3(-2.0, -2.0, -2.0));
    STATIC_ASSERT_TRUE(point2(1.0, 2.0) - point3(3.0, 4.0, 5.0) == vector3(-2.0, -2.0, -5.0));
    STATIC_ASSERT_TRUE(point3(1.0, 2.0, 3.0) - point2(3.0, 4.0) == vector3(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - point2(3.0, 4.0)), vector3>);
    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - point3(3.0, 4.0, 5.0)), vector3>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - point3(3.0, 4.0, 5.0)), vector3>);

    STATIC_ASSERT_TRUE(point3(1.0, 2.0, 3.0) - vector3(3.0, 4.0, 5.0) == point3(-2.0, -2.0, -2.0));
    STATIC_ASSERT_TRUE(point2(1.0, 2.0) - vector3(3.0, 4.0, 5.0) == point3(-2.0, -2.0, -5.0));
    STATIC_ASSERT_TRUE(point3(1.0, 2.0, 3.0) - vector2(3.0, 4.0) == point3(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - vector2(3.0, 4.0)), point3>);
    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - vector3(3.0, 4.0, 5.0)), point3>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - vector3(3.0, 4.0, 5.0)), point3>);
}
