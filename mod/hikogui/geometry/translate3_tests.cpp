// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "translate3.hpp"
#include "transform.hpp"
#include "../utility/utility.hpp"
#include "../test.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>



using namespace hi;

TEST(translate3, translate_vector)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * vector3(1.0, 2.0, 3.0)), vector3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * vector2(1.0, 2.0)), vector2>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * vector3(1.0, 2.0, 3.0)), vector3>);

    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * vector3(1.0, 2.0, 3.0) == vector3(1.0, 2.0, 3.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * vector2(1.0, 2.0) == vector2(1.0, 2.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * vector3(1.0, 2.0, 3.0) == vector3(1.0, 2.0, 3.0));
}

TEST(translate3, translate_point)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)), point3>);

    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * point3(1.0, 2.0, 3.0) == point3(5.0, 8.0, 3.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * point2(1.0, 2.0) == point3(5.0, 8.0, 8.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0) == point3(5.0, 8.0, 11.0));
}

TEST(translate3, translate_translate)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * translate3(1.0, 2.0, 3.0)), translate3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * translate2(1.0, 2.0)), translate3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * translate3(1.0, 2.0, 3.0)), translate3>);

    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * translate3(1.0, 2.0, 3.0) == translate3(5.0, 8.0, 3.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * translate2(1.0, 2.0) == translate3(5.0, 8.0, 8.0));
    STATIC_ASSERT_TRUE(translate3(4.0, 6.0, 8.0) * translate3(1.0, 2.0, 3.0) == translate3(5.0, 8.0, 11.0));
}
