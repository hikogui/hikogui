// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "translate2.hpp"
#include "transform.hpp"
#include "../utility/utility.hpp"
#include "../test.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>



using namespace hi;

TEST(translate2, translate_vector)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * vector2(1.0, 2.0)), vector2>);

    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * vector2(1.0, 2.0) == vector2(1.0, 2.0));
}

TEST(translate2, translate_point)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * point2(1.0, 2.0)), point2>);

    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * point2(1.0, 2.0) == point2(5.0, 8.0));
}

TEST(translate2, translate_translate)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * translate2(1.0, 2.0)), translate2>);

    STATIC_ASSERT_TRUE(translate2(4.0, 6.0) * translate2(1.0, 2.0) == translate2(5.0, 8.0));
}
