// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "point2.hpp"
#include "../utility/utility.hpp"
#include "../test.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>



using namespace hi;

TEST(point2, compare)
{
    STATIC_ASSERT_FALSE(point2(1.0, 2.0) == point2(3.0, 4.0));
    STATIC_ASSERT_FALSE(point2(1.0, 2.0) == point2(1.0, 4.0));
    STATIC_ASSERT_FALSE(point2(1.0, 2.0) == point2(3.0, 2.0));
    STATIC_ASSERT_TRUE(point2(1.0, 2.0) == point2(1.0, 2.0));

    STATIC_ASSERT_TRUE(point2(1.0, 2.0) != point2(3.0, 4.0));
    STATIC_ASSERT_TRUE(point2(1.0, 2.0) != point2(1.0, 4.0));
    STATIC_ASSERT_TRUE(point2(1.0, 2.0) != point2(3.0, 2.0));
    STATIC_ASSERT_FALSE(point2(1.0, 2.0) != point2(1.0, 2.0));
}

TEST(point2, adding)
{
    STATIC_ASSERT_TRUE(point2(1.0, 2.0) + vector2(3.0, 4.0) == point2(4.0, 6.0));

    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) + vector2(3.0, 4.0)), point2>);

    STATIC_ASSERT_TRUE(vector2(1.0, 2.0) + point2(3.0, 4.0) == point2(4.0, 6.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + point2(3.0, 4.0)), point2>);
}

TEST(point2, subtracting)
{
    STATIC_ASSERT_TRUE(point2(1.0, 2.0) - point2(3.0, 4.0) == vector2(-2.0, -2.0));

    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - point2(3.0, 4.0)), vector2>);

    STATIC_ASSERT_TRUE(point2(1.0, 2.0) - vector2(3.0, 4.0) == point2(-2.0, -2.0));

    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - vector2(3.0, 4.0)), point2>);
}
