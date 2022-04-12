// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/geometry/translate.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi::geo;

TEST(geometry, translate_vector)
{
    static_assert(std::is_same_v<decltype(translate<2>(4.0, 6.0) * vector<2>(1.0, 2.0)), vector<2>>);
    static_assert(std::is_same_v<decltype(translate<2>(4.0, 6.0) * vector<3>(1.0, 2.0, 3.0)), vector<3>>);
    static_assert(std::is_same_v<decltype(translate<3>(4.0, 6.0, 8.0) * vector<2>(1.0, 2.0)), vector<2>>);
    static_assert(std::is_same_v<decltype(translate<3>(4.0, 6.0, 8.0) * vector<3>(1.0, 2.0, 3.0)), vector<3>>);

    static_assert(translate<2>(4.0, 6.0) * vector<2>(1.0, 2.0) == vector<2>(1.0, 2.0));
    static_assert(translate<2>(4.0, 6.0) * vector<3>(1.0, 2.0, 3.0) == vector<3>(1.0, 2.0, 3.0));
    static_assert(translate<3>(4.0, 6.0, 8.0) * vector<2>(1.0, 2.0) == vector<2>(1.0, 2.0));
    static_assert(translate<3>(4.0, 6.0, 8.0) * vector<3>(1.0, 2.0, 3.0) == vector<3>(1.0, 2.0, 3.0));

    ASSERT_TRUE(translate<2>(4.0, 6.0) * vector<2>(1.0, 2.0) == vector<2>(1.0, 2.0));
    ASSERT_TRUE(translate<2>(4.0, 6.0) * vector<3>(1.0, 2.0, 3.0) == vector<3>(1.0, 2.0, 3.0));
    ASSERT_TRUE(translate<3>(4.0, 6.0, 8.0) * vector<2>(1.0, 2.0) == vector<2>(1.0, 2.0));
    ASSERT_TRUE(translate<3>(4.0, 6.0, 8.0) * vector<3>(1.0, 2.0, 3.0) == vector<3>(1.0, 2.0, 3.0));
}

TEST(geometry, translate_point)
{
    static_assert(std::is_same_v<decltype(translate<2>(4.0, 6.0) * point<2>(1.0, 2.0)), point<2>>);
    static_assert(std::is_same_v<decltype(translate<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0)), point<3>>);
    static_assert(std::is_same_v<decltype(translate<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0)), point<3>>);
    static_assert(std::is_same_v<decltype(translate<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0)), point<3>>);

    static_assert(translate<2>(4.0, 6.0) * point<2>(1.0, 2.0) == point<2>(5.0, 8.0));
    static_assert(translate<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0) == point<3>(5.0, 8.0, 3.0));
    static_assert(translate<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0) == point<3>(5.0, 8.0, 8.0));
    static_assert(translate<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0) == point<3>(5.0, 8.0, 11.0));

    ASSERT_TRUE(translate<2>(4.0, 6.0) * point<2>(1.0, 2.0) == point<2>(5.0, 8.0));
    ASSERT_TRUE(translate<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0) == point<3>(5.0, 8.0, 3.0));
    ASSERT_TRUE(translate<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0) == point<3>(5.0, 8.0, 8.0));
    ASSERT_TRUE(translate<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0) == point<3>(5.0, 8.0, 11.0));
}

TEST(geometry, translate_identity)
{
    static_assert(std::is_same_v<decltype(translate<2>(1.0, 2.0) * identity()), translate<2>>);
    static_assert(std::is_same_v<decltype(translate<3>(1.0, 2.0, 3.0) * identity()), translate<3>>);

    static_assert(translate<2>(1.0, 2.0) * identity() == translate<2>(1.0, 2.0));
    static_assert(translate<3>(1.0, 2.0, 3.0) * identity() == translate<3>(1.0, 2.0, 3.0));

    ASSERT_TRUE(translate<2>(1.0, 2.0) * identity() == translate<2>(1.0, 2.0));
    ASSERT_TRUE(translate<3>(1.0, 2.0, 3.0) * identity() == translate<3>(1.0, 2.0, 3.0));
}

TEST(geometry, translate_translate)
{
    static_assert(std::is_same_v<decltype(translate<2>(4.0, 6.0) * translate<2>(1.0, 2.0)), translate<2>>);
    static_assert(std::is_same_v<decltype(translate<2>(4.0, 6.0) * translate<3>(1.0, 2.0, 3.0)), translate<3>>);
    static_assert(std::is_same_v<decltype(translate<3>(4.0, 6.0, 8.0) * translate<2>(1.0, 2.0)), translate<3>>);
    static_assert(std::is_same_v<decltype(translate<3>(4.0, 6.0, 8.0) * translate<3>(1.0, 2.0, 3.0)), translate<3>>);

    static_assert(translate<2>(4.0, 6.0) * translate<2>(1.0, 2.0) == translate<2>(5.0, 8.0));
    static_assert(translate<2>(4.0, 6.0) * translate<3>(1.0, 2.0, 3.0) == translate<3>(5.0, 8.0, 3.0));
    static_assert(translate<3>(4.0, 6.0, 8.0) * translate<2>(1.0, 2.0) == translate<3>(5.0, 8.0, 8.0));
    static_assert(translate<3>(4.0, 6.0, 8.0) * translate<3>(1.0, 2.0, 3.0) == translate<3>(5.0, 8.0, 11.0));

    ASSERT_TRUE(translate<2>(4.0, 6.0) * translate<2>(1.0, 2.0) == translate<2>(5.0, 8.0));
    ASSERT_TRUE(translate<2>(4.0, 6.0) * translate<3>(1.0, 2.0, 3.0) == translate<3>(5.0, 8.0, 3.0));
    ASSERT_TRUE(translate<3>(4.0, 6.0, 8.0) * translate<2>(1.0, 2.0) == translate<3>(5.0, 8.0, 8.0));
    ASSERT_TRUE(translate<3>(4.0, 6.0, 8.0) * translate<3>(1.0, 2.0, 3.0) == translate<3>(5.0, 8.0, 11.0));
}
