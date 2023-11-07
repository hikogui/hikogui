// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "scale3.hpp"
#include "transform.hpp"
#include "../utility/utility.hpp"
#include "../test.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>



using namespace hi;

TEST(scale3, scale_vector)
{
    static_assert(std::is_same_v<decltype(scale2(4.0, 6.0) * vector3(1.0, 2.0, 3.0)), vector3>);
    static_assert(std::is_same_v<decltype(scale3(4.0, 6.0, 8.0) * vector2(1.0, 2.0)), vector2>);
    static_assert(std::is_same_v<decltype(scale3(4.0, 6.0, 8.0) * vector3(1.0, 2.0, 3.0)), vector3>);

    STATIC_ASSERT_TRUE(scale2(4.0, 6.0) * vector3(1.0, 2.0, 3.0) == vector3(4.0, 12.0, 3.0));
    STATIC_ASSERT_TRUE(scale3(4.0, 6.0, 8.0) * vector2(1.0, 2.0) == vector2(4.0, 12.0));
    STATIC_ASSERT_TRUE(scale3(4.0, 6.0, 8.0) * vector3(1.0, 2.0, 3.0) == vector3(4, 12.0, 24.0));
}

TEST(scale3, scale_point)
{
    static_assert(std::is_same_v<decltype(scale2(4.0, 6.0) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype(scale3(4.0, 6.0, 8.0) * point2(1.0, 2.0)), point2>);
    static_assert(std::is_same_v<decltype(scale3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)), point3>);

    STATIC_ASSERT_TRUE(scale2(4.0, 6.0) * point3(1.0, 2.0, 3.0) == point3(4.0, 12.0, 3.0));
    STATIC_ASSERT_TRUE(scale3(4.0, 6.0, 8.0) * point2(1.0, 2.0) == point2(4.0, 12.0));
    STATIC_ASSERT_TRUE(scale3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0) == point3(4, 12.0, 24.0));
}

TEST(scale3, scale_scale)
{
    static_assert(std::is_same_v<decltype(scale2(4.0, 6.0) * scale3(1.0, 2.0, 3.0)), scale3>);
    static_assert(std::is_same_v<decltype(scale3(4.0, 6.0, 8.0) * scale2(1.0, 2.0)), scale3>);
    static_assert(std::is_same_v<decltype(scale3(4.0, 6.0, 8.0) * scale3(1.0, 2.0, 3.0)), scale3>);

    STATIC_ASSERT_TRUE(scale2(4.0, 6.0) * scale3(1.0, 2.0, 3.0) == scale3(4.0, 12.0, 3.0));
    STATIC_ASSERT_TRUE(scale3(4.0, 6.0, 8.0) * scale2(1.0, 2.0) == scale3(4.0, 12.0, 8.0));
    STATIC_ASSERT_TRUE(scale3(4.0, 6.0, 8.0) * scale3(1.0, 2.0, 3.0) == scale3(4, 12.0, 24.0));
}
