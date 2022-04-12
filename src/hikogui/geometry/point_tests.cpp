// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/geometry/point.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi::geo;

TEST(point, compare)
{
    ASSERT_FALSE(point<2>(1.0, 2.0) == point<2>(3.0, 4.0));
    ASSERT_FALSE(point<2>(1.0, 2.0) == point<2>(1.0, 4.0));
    ASSERT_FALSE(point<2>(1.0, 2.0) == point<2>(3.0, 2.0));
    ASSERT_TRUE(point<2>(1.0, 2.0) == point<2>(1.0, 2.0));

    ASSERT_TRUE(point<2>(1.0, 2.0) != point<2>(3.0, 4.0));
    ASSERT_TRUE(point<2>(1.0, 2.0) != point<2>(1.0, 4.0));
    ASSERT_TRUE(point<2>(1.0, 2.0) != point<2>(3.0, 2.0));
    ASSERT_FALSE(point<2>(1.0, 2.0) != point<2>(1.0, 2.0));

    ASSERT_FALSE(point<3>(1.0, 2.0, 3.0) == point<3>(3.0, 4.0, 5.0));
    ASSERT_FALSE(point<3>(1.0, 2.0, 3.0) == point<3>(1.0, 4.0, 5.0));
    ASSERT_FALSE(point<3>(1.0, 2.0, 3.0) == point<3>(3.0, 2.0, 5.0));
    ASSERT_TRUE(point<3>(1.0, 2.0, 3.0) == point<3>(1.0, 2.0, 3.0));

    ASSERT_FALSE(point<2>(1.0, 2.0) == point<3>(3.0, 4.0, 5.0));
    ASSERT_FALSE(point<2>(1.0, 2.0) == point<3>(1.0, 4.0, 5.0));
    ASSERT_FALSE(point<2>(1.0, 2.0) == point<3>(3.0, 2.0, 5.0));
    ASSERT_FALSE(point<2>(1.0, 2.0) == point<3>(1.0, 2.0, 3.0));
    ASSERT_TRUE(point<2>(1.0, 2.0) == point<3>(1.0, 2.0, 0.0));
}

TEST(point, adding)
{
    ASSERT_EQ(point<2>(1.0, 2.0) + vector<2>(3.0, 4.0), point<2>(4.0, 6.0));
    ASSERT_EQ(point<3>(1.0, 2.0, 3.0) + vector<3>(3.0, 4.0, 5.0), point<3>(4.0, 6.0, 8.0));
    ASSERT_EQ(point<2>(1.0, 2.0) + vector<3>(3.0, 4.0, 5.0), point<3>(4.0, 6.0, 5.0));
    ASSERT_EQ(point<3>(1.0, 2.0, 3.0) + vector<2>(3.0, 4.0), point<3>(4.0, 6.0, 3.0));

    static_assert(point<2>(1.0, 2.0) + vector<2>(3.0, 4.0) == point<2>(4.0, 6.0));
    static_assert(point<3>(1.0, 2.0, 3.0) + vector<3>(3.0, 4.0, 5.0) == point<3>(4.0, 6.0, 8.0));
    static_assert(point<2>(1.0, 2.0) + vector<3>(3.0, 4.0, 5.0) == point<3>(4.0, 6.0, 5.0));
    static_assert(point<3>(1.0, 2.0, 3.0) + vector<2>(3.0, 4.0) == point<3>(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(point<2>(1.0, 2.0) + vector<2>(3.0, 4.0)), point<2>>);
    static_assert(std::is_same_v<decltype(point<3>(1.0, 2.0, 3.0) + vector<2>(3.0, 4.0)), point<3>>);
    static_assert(std::is_same_v<decltype(point<2>(1.0, 2.0) + vector<3>(3.0, 4.0, 5.0)), point<3>>);
    static_assert(std::is_same_v<decltype(point<3>(1.0, 2.0, 3.0) + vector<3>(3.0, 4.0, 5.0)), point<3>>);

    ASSERT_EQ(vector<2>(1.0, 2.0) + point<2>(3.0, 4.0), point<2>(4.0, 6.0));
    ASSERT_EQ(vector<3>(1.0, 2.0, 3.0) + point<3>(3.0, 4.0, 5.0), point<3>(4.0, 6.0, 8.0));
    ASSERT_EQ(vector<2>(1.0, 2.0) + point<3>(3.0, 4.0, 5.0), point<3>(4.0, 6.0, 5.0));
    ASSERT_EQ(vector<3>(1.0, 2.0, 3.0) + point<2>(3.0, 4.0), point<3>(4.0, 6.0, 3.0));

    static_assert(vector<2>(1.0, 2.0) + point<2>(3.0, 4.0) == point<2>(4.0, 6.0));
    static_assert(vector<3>(1.0, 2.0, 3.0) + point<3>(3.0, 4.0, 5.0) == point<3>(4.0, 6.0, 8.0));
    static_assert(vector<2>(1.0, 2.0) + point<3>(3.0, 4.0, 5.0) == point<3>(4.0, 6.0, 5.0));
    static_assert(vector<3>(1.0, 2.0, 3.0) + point<2>(3.0, 4.0) == point<3>(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(vector<2>(1.0, 2.0) + point<2>(3.0, 4.0)), point<2>>);
    static_assert(std::is_same_v<decltype(vector<3>(1.0, 2.0, 3.0) + point<2>(3.0, 4.0)), point<3>>);
    static_assert(std::is_same_v<decltype(vector<2>(1.0, 2.0) + point<3>(3.0, 4.0, 5.0)), point<3>>);
    static_assert(std::is_same_v<decltype(vector<3>(1.0, 2.0, 3.0) + point<3>(3.0, 4.0, 5.0)), point<3>>);
}

TEST(point, subtracting)
{
    ASSERT_EQ(point<2>(1.0, 2.0) - point<2>(3.0, 4.0), vector<2>(-2.0, -2.0));
    ASSERT_EQ(point<3>(1.0, 2.0, 3.0) - point<3>(3.0, 4.0, 5.0), vector<3>(-2.0, -2.0, -2.0));
    ASSERT_EQ(point<2>(1.0, 2.0) - point<3>(3.0, 4.0, 5.0), vector<3>(-2.0, -2.0, -5.0));
    ASSERT_EQ(point<3>(1.0, 2.0, 3.0) - point<2>(3.0, 4.0), vector<3>(-2.0, -2.0, 3.0));

    static_assert(point<2>(1.0, 2.0) - point<2>(3.0, 4.0) == vector<2>(-2.0, -2.0));
    static_assert(point<3>(1.0, 2.0, 3.0) - point<3>(3.0, 4.0, 5.0) == vector<3>(-2.0, -2.0, -2.0));
    static_assert(point<2>(1.0, 2.0) - point<3>(3.0, 4.0, 5.0) == vector<3>(-2.0, -2.0, -5.0));
    static_assert(point<3>(1.0, 2.0, 3.0) - point<2>(3.0, 4.0) == vector<3>(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(point<2>(1.0, 2.0) - point<2>(3.0, 4.0)), vector<2>>);
    static_assert(std::is_same_v<decltype(point<3>(1.0, 2.0, 3.0) - point<2>(3.0, 4.0)), vector<3>>);
    static_assert(std::is_same_v<decltype(point<2>(1.0, 2.0) - point<3>(3.0, 4.0, 5.0)), vector<3>>);
    static_assert(std::is_same_v<decltype(point<3>(1.0, 2.0, 3.0) - point<3>(3.0, 4.0, 5.0)), vector<3>>);

    ASSERT_EQ(point<2>(1.0, 2.0) - vector<2>(3.0, 4.0), point<2>(-2.0, -2.0));
    ASSERT_EQ(point<3>(1.0, 2.0, 3.0) - vector<3>(3.0, 4.0, 5.0), point<3>(-2.0, -2.0, -2.0));
    ASSERT_EQ(point<2>(1.0, 2.0) - vector<3>(3.0, 4.0, 5.0), point<3>(-2.0, -2.0, -5.0));
    ASSERT_EQ(point<3>(1.0, 2.0, 3.0) - vector<2>(3.0, 4.0), point<3>(-2.0, -2.0, 3.0));

    static_assert(point<2>(1.0, 2.0) - vector<2>(3.0, 4.0) == point<2>(-2.0, -2.0));
    static_assert(point<3>(1.0, 2.0, 3.0) - vector<3>(3.0, 4.0, 5.0) == point<3>(-2.0, -2.0, -2.0));
    static_assert(point<2>(1.0, 2.0) - vector<3>(3.0, 4.0, 5.0) == point<3>(-2.0, -2.0, -5.0));
    static_assert(point<3>(1.0, 2.0, 3.0) - vector<2>(3.0, 4.0) == point<3>(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(point<2>(1.0, 2.0) - vector<2>(3.0, 4.0)), point<2>>);
    static_assert(std::is_same_v<decltype(point<3>(1.0, 2.0, 3.0) - vector<2>(3.0, 4.0)), point<3>>);
    static_assert(std::is_same_v<decltype(point<2>(1.0, 2.0) - vector<3>(3.0, 4.0, 5.0)), point<3>>);
    static_assert(std::is_same_v<decltype(point<3>(1.0, 2.0, 3.0) - vector<3>(3.0, 4.0, 5.0)), point<3>>);
}
