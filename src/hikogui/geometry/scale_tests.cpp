// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/geometry/scale.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi::geo;

TEST(geometry, scale_vector)
{
    static_assert(std::is_same_v<decltype(scale<2>(4.0, 6.0) * vector<2>(1.0, 2.0)), vector<2>>);
    static_assert(std::is_same_v<decltype(scale<2>(4.0, 6.0) * vector<3>(1.0, 2.0, 3.0)), vector<3>>);
    static_assert(std::is_same_v<decltype(scale<3>(4.0, 6.0, 8.0) * vector<2>(1.0, 2.0)), vector<2>>);
    static_assert(std::is_same_v<decltype(scale<3>(4.0, 6.0, 8.0) * vector<3>(1.0, 2.0, 3.0)), vector<3>>);

    static_assert(scale<2>(4.0, 6.0) * vector<2>(1.0, 2.0) == vector<2>(4.0, 12.0));
    static_assert(scale<2>(4.0, 6.0) * vector<3>(1.0, 2.0, 3.0) == vector<3>(4.0, 12.0, 3.0));
    static_assert(scale<3>(4.0, 6.0, 8.0) * vector<2>(1.0, 2.0) == vector<2>(4.0, 12.0));
    static_assert(scale<3>(4.0, 6.0, 8.0) * vector<3>(1.0, 2.0, 3.0) == vector<3>(4, 12.0, 24.0));

    ASSERT_TRUE(scale<2>(4.0, 6.0) * vector<2>(1.0, 2.0) == vector<2>(4.0, 12.0));
    ASSERT_TRUE(scale<2>(4.0, 6.0) * vector<3>(1.0, 2.0, 3.0) == vector<3>(4.0, 12.0, 3.0));
    ASSERT_TRUE(scale<3>(4.0, 6.0, 8.0) * vector<2>(1.0, 2.0) == vector<2>(4.0, 12.0));
    ASSERT_TRUE(scale<3>(4.0, 6.0, 8.0) * vector<3>(1.0, 2.0, 3.0) == vector<3>(4, 12.0, 24.0));
}

TEST(geometry, scale_point)
{
    static_assert(std::is_same_v<decltype(scale<2>(4.0, 6.0) * point<2>(1.0, 2.0)), point<2>>);
    static_assert(std::is_same_v<decltype(scale<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0)), point<3>>);
    static_assert(std::is_same_v<decltype(scale<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0)), point<2>>);
    static_assert(std::is_same_v<decltype(scale<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0)), point<3>>);

    static_assert(scale<2>(4.0, 6.0) * point<2>(1.0, 2.0) == point<2>(4.0, 12.0));
    static_assert(scale<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0) == point<3>(4.0, 12.0, 3.0));
    static_assert(scale<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0) == point<2>(4.0, 12.0));
    static_assert(scale<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0) == point<3>(4, 12.0, 24.0));

    ASSERT_TRUE(scale<2>(4.0, 6.0) * point<2>(1.0, 2.0) == point<2>(4.0, 12.0));
    ASSERT_TRUE(scale<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0) == point<3>(4.0, 12.0, 3.0));
    ASSERT_TRUE(scale<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0) == point<2>(4.0, 12.0));
    ASSERT_TRUE(scale<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0) == point<3>(4, 12.0, 24.0));
}

TEST(geometry, scale_identity)
{
    static_assert(std::is_same_v<decltype(scale<2>(1.0, 2.0) * identity()), scale<2>>);
    static_assert(std::is_same_v<decltype(scale<3>(1.0, 2.0, 3.0) * identity()), scale<3>>);

    static_assert(scale<2>(1.0, 2.0) * identity() == scale<2>(1.0, 2.0));
    static_assert(scale<3>(1.0, 2.0, 3.0) * identity() == scale<3>(1.0, 2.0, 3.0));

    ASSERT_TRUE(scale<2>(1.0, 2.0) * identity() == scale<2>(1.0, 2.0));
    ASSERT_TRUE(scale<3>(1.0, 2.0, 3.0) * identity() == scale<3>(1.0, 2.0, 3.0));
}

TEST(geometry, scale_scale)
{
    static_assert(std::is_same_v<decltype(scale<2>(4.0, 6.0) * scale<2>(1.0, 2.0)), scale<2>>);
    static_assert(std::is_same_v<decltype(scale<2>(4.0, 6.0) * scale<3>(1.0, 2.0, 3.0)), scale<3>>);
    static_assert(std::is_same_v<decltype(scale<3>(4.0, 6.0, 8.0) * scale<2>(1.0, 2.0)), scale<3>>);
    static_assert(std::is_same_v<decltype(scale<3>(4.0, 6.0, 8.0) * scale<3>(1.0, 2.0, 3.0)), scale<3>>);

    static_assert(scale<2>(4.0, 6.0) * scale<2>(1.0, 2.0) == scale<2>(4.0, 12.0));
    static_assert(scale<2>(4.0, 6.0) * scale<3>(1.0, 2.0, 3.0) == scale<3>(4.0, 12.0, 3.0));
    static_assert(scale<3>(4.0, 6.0, 8.0) * scale<2>(1.0, 2.0) == scale<3>(4.0, 12.0, 8.0));
    static_assert(scale<3>(4.0, 6.0, 8.0) * scale<3>(1.0, 2.0, 3.0) == scale<3>(4, 12.0, 24.0));

    ASSERT_TRUE(scale<2>(4.0, 6.0) * scale<2>(1.0, 2.0) == scale<2>(4.0, 12.0));
    ASSERT_TRUE(scale<2>(4.0, 6.0) * scale<3>(1.0, 2.0, 3.0) == scale<3>(4.0, 12.0, 3.0));
    ASSERT_TRUE(scale<3>(4.0, 6.0, 8.0) * scale<2>(1.0, 2.0) == scale<3>(4.0, 12.0, 8.0));
    ASSERT_TRUE(scale<3>(4.0, 6.0, 8.0) * scale<3>(1.0, 2.0, 3.0) == scale<3>(4, 12.0, 24.0));
}
