// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "transform.hpp"
#include "../utility/utility.hpp"
#include "../test.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>



using namespace hi;


TEST(gtransform, translate_scale_point)
{
    static_assert(std::is_same_v<decltype(translate2(-3, -4) * (scale2(4.0, 6.0) * point2(1.0, 2.0))), point2>);
    static_assert(std::is_same_v<decltype(translate2(-3, -4) * (scale2(4.0, 6.0) * point3(1.0, 2.0, 3.0))), point3>);
    static_assert(std::is_same_v<decltype(translate2(-3, -4) * (scale3(4.0, 6.0, 8.0) * point2(1.0, 2.0))), point2>);
    static_assert(std::is_same_v<decltype(translate2(-3, -4) * (scale3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0))), point3>);
    static_assert(std::is_same_v<decltype(translate3(-3, -4, -5) * (scale2(4.0, 6.0) * point2(1.0, 2.0))), point3>);
    static_assert(std::is_same_v<decltype(translate3(-3, -4, -5) * (scale2(4.0, 6.0) * point3(1.0, 2.0, 3.0))), point3>);
    static_assert(std::is_same_v<decltype(translate3(-3, -4, -5) * (scale3(4.0, 6.0, 8.0) * point2(1.0, 2.0))), point3>);
    static_assert(
        std::is_same_v<decltype(translate3(-3, -4, -5) * (scale3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0))), point3>);

    STATIC_ASSERT_TRUE(translate2(-3, -4) * (scale2(4.0, 6.0) * point2(1.0, 2.0)) == point2(1.0, 8.0));
    STATIC_ASSERT_TRUE(translate2(-3, -4) * (scale2(4.0, 6.0) * point3(1.0, 2.0, 3.0)) == point3(1.0, 8.0, 3.0));
    STATIC_ASSERT_TRUE(translate2(-3, -4) * (scale3(4.0, 6.0, 8.0) * point2(1.0, 2.0)) == point2(1.0, 8.0));
    STATIC_ASSERT_TRUE(translate2(-3, -4) * (scale3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)) == point3(1, 8.0, 24.0));
    STATIC_ASSERT_TRUE(translate3(-3, -4, -5) * (scale2(4.0, 6.0) * point2(1.0, 2.0)) == point3(1.0, 8.0, -5));
    STATIC_ASSERT_TRUE(translate3(-3, -4, -5) * (scale2(4.0, 6.0) * point3(1.0, 2.0, 3.0)) == point3(1.0, 8.0, -2.0));
    STATIC_ASSERT_TRUE(translate3(-3, -4, -5) * (scale3(4.0, 6.0, 8.0) * point2(1.0, 2.0)) == point3(1.0, 8.0, -5));
    STATIC_ASSERT_TRUE(translate3(-3, -4, -5) * (scale3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)) == point3(1, 8.0, 19.0));

    static_assert(std::is_same_v<decltype((translate2(-3, -4) * scale2(4.0, 6.0)) * point2(1.0, 2.0)), point2>);
    static_assert(std::is_same_v<decltype((translate2(-3, -4) * scale2(4.0, 6.0)) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype((translate2(-3, -4) * scale3(4.0, 6.0, 8.0)) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype((translate2(-3, -4) * scale3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype((translate3(-3, -4, -5) * scale2(4.0, 6.0)) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype((translate3(-3, -4, -5) * scale2(4.0, 6.0)) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype((translate3(-3, -4, -5) * scale3(4.0, 6.0, 8.0)) * point2(1.0, 2.0)), point3>);
    static_assert(
        std::is_same_v<decltype((translate3(-3, -4, -5) * scale3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0)), point3>);

    STATIC_ASSERT_TRUE((translate2(-3, -4) * scale2(4.0, 6.0)) * point3(1.0, 2.0, 3.0) == point3(1.0, 8.0, 3.0));
    STATIC_ASSERT_TRUE((translate2(-3, -4) * scale3(4.0, 6.0, 8.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, 0.0));
    STATIC_ASSERT_TRUE((translate2(-3, -4) * scale3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0) == point3(1, 8.0, 24.0));
    STATIC_ASSERT_TRUE((translate2(-3, -4) * scale2(4.0, 6.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, 0.0));
    STATIC_ASSERT_TRUE((translate3(-3, -4, -5) * scale2(4.0, 6.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, -5));
    STATIC_ASSERT_TRUE((translate3(-3, -4, -5) * scale2(4.0, 6.0)) * point3(1.0, 2.0, 3.0) == point3(1.0, 8.0, -2.0));
    STATIC_ASSERT_TRUE((translate3(-3, -4, -5) * scale3(4.0, 6.0, 8.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, -5));
    STATIC_ASSERT_TRUE((translate3(-3, -4, -5) * scale3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0) == point3(1, 8.0, 19.0));
}
