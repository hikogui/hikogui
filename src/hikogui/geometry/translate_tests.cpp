// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "translate.hpp"
#include "../utility/module.hpp"
#include "../utility/test.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi;
using namespace hi::geo;

TEST(geometry, translate_vector)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * vector2(1.0, 2.0)), vector2>);
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * vector3(1.0, 2.0, 3.0)), vector3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * vector2(1.0, 2.0)), vector2>);
    static_assert(
        std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * vector3(1.0, 2.0, 3.0)), vector3>);

    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * vector2(1.0, 2.0) == vector2(1.0, 2.0));
    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * vector3(1.0, 2.0, 3.0) == vector3(1.0, 2.0, 3.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * vector2(1.0, 2.0) == vector2(1.0, 2.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * vector3(1.0, 2.0, 3.0) == vector3(1.0, 2.0, 3.0));
}

TEST(geometry, translate_point)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * point2(1.0, 2.0)), point2>);
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)), point3>);

    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * point2(1.0, 2.0) == point2(5.0, 8.0));
    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * point3(1.0, 2.0, 3.0) == point3(5.0, 8.0, 3.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * point2(1.0, 2.0) == point3(5.0, 8.0, 8.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0) == point3(5.0, 8.0, 11.0));
}

TEST(geometry, translate_identity)
{
    static_assert(std::is_same_v<decltype(translate2(1.0, 2.0) * identity()), translate2>);
    static_assert(std::is_same_v<decltype(translate3(1.0, 2.0, 3.0) * identity()), translate3>);

    STATIC_ASSERT_TRUE(translate2(1.0, 2.0) * identity() == translate2(1.0, 2.0));
    STATIC_ASSERT_TRUE(translate3(1.0, 2.0, 3.0) * identity() == translate3(1.0, 2.0, 3.0));
}

TEST(geometry, translate_translate)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * translate2(1.0, 2.0)), translate2>);
    static_assert(
        std::is_same_v<decltype(translate2(4.0, 6.0) * translate3(1.0, 2.0, 3.0)), translate3>);
    static_assert(
        std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * translate2(1.0, 2.0)), translate3>);
    static_assert(
        std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * translate3(1.0, 2.0, 3.0)), translate3>);

    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * translate2(1.0, 2.0) == translate2(5.0, 8.0));
    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * translate3(1.0, 2.0, 3.0) == translate3(5.0, 8.0, 3.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * translate2(1.0, 2.0) == translate3(5.0, 8.0, 8.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * translate3(1.0, 2.0, 3.0) == translate3(5.0, 8.0, 11.0));
}
