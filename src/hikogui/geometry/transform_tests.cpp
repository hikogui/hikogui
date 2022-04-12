// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/geometry/transform.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi::geo;

TEST(geometry, identity_translate)
{
    static_assert(std::is_same_v<decltype(identity() * translate<2>(1.0, 2.0)), translate<2>>);
    static_assert(std::is_same_v<decltype(identity() * translate<3>(1.0, 2.0, 3.0)), translate<3>>);

    static_assert(identity() * translate<2>(1.0, 2.0) == translate<2>(1.0, 2.0));
    static_assert(identity() * translate<3>(1.0, 2.0, 3.0) == translate<3>(1.0, 2.0, 3.0));

    ASSERT_TRUE(identity() * translate<2>(1.0, 2.0) == translate<2>(1.0, 2.0));
    ASSERT_TRUE(identity() * translate<3>(1.0, 2.0, 3.0) == translate<3>(1.0, 2.0, 3.0));
}

TEST(geometry, identity_scale)
{
    static_assert(std::is_same_v<decltype(identity() * scale<2>(1.0, 2.0)), scale<2>>);
    static_assert(std::is_same_v<decltype(identity() * scale<3>(1.0, 2.0, 3.0)), scale<3>>);

    static_assert(identity() * scale<2>(1.0, 2.0) == scale<2>(1.0, 2.0));
    static_assert(identity() * scale<3>(1.0, 2.0, 3.0) == scale<3>(1.0, 2.0, 3.0));

    ASSERT_TRUE(identity() * scale<2>(1.0, 2.0) == scale<2>(1.0, 2.0));
    ASSERT_TRUE(identity() * scale<3>(1.0, 2.0, 3.0) == scale<3>(1.0, 2.0, 3.0));
}

TEST(geometry, translate_scale_point)
{
    static_assert(std::is_same_v<decltype(translate<2>(-3, -4) * (scale<2>(4.0, 6.0) * point<2>(1.0, 2.0))), point<2>>);
    static_assert(std::is_same_v<decltype(translate<2>(-3, -4) * (scale<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0))), point<3>>);
    static_assert(std::is_same_v<decltype(translate<2>(-3, -4) * (scale<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0))), point<2>>);
    static_assert(std::is_same_v<decltype(translate<2>(-3, -4) * (scale<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0))), point<3>>);
    static_assert(std::is_same_v<decltype(translate<3>(-3, -4, -5) * (scale<2>(4.0, 6.0) * point<2>(1.0, 2.0))), point<3>>);
    static_assert(std::is_same_v<decltype(translate<3>(-3, -4, -5) * (scale<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0))), point<3>>);
    static_assert(std::is_same_v<decltype(translate<3>(-3, -4, -5) * (scale<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0))), point<3>>);
    static_assert(
        std::is_same_v<decltype(translate<3>(-3, -4, -5) * (scale<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0))), point<3>>);

    static_assert(translate<2>(-3, -4) * (scale<2>(4.0, 6.0) * point<2>(1.0, 2.0)) == point<2>(1.0, 8.0));
    static_assert(translate<2>(-3, -4) * (scale<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0)) == point<3>(1.0, 8.0, 3.0));
    static_assert(translate<2>(-3, -4) * (scale<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0)) == point<2>(1.0, 8.0));
    static_assert(translate<2>(-3, -4) * (scale<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0)) == point<3>(1, 8.0, 24.0));
    static_assert(translate<3>(-3, -4, -5) * (scale<2>(4.0, 6.0) * point<2>(1.0, 2.0)) == point<3>(1.0, 8.0, -5));
    static_assert(translate<3>(-3, -4, -5) * (scale<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0)) == point<3>(1.0, 8.0, -2.0));
    static_assert(translate<3>(-3, -4, -5) * (scale<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0)) == point<3>(1.0, 8.0, -5));
    static_assert(translate<3>(-3, -4, -5) * (scale<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0)) == point<3>(1, 8.0, 19.0));

    ASSERT_TRUE(translate<2>(-3, -4) * (scale<2>(4.0, 6.0) * point<2>(1.0, 2.0)) == point<2>(1.0, 8.0));
    ASSERT_TRUE(translate<2>(-3, -4) * (scale<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0)) == point<3>(1.0, 8.0, 3.0));
    ASSERT_TRUE(translate<2>(-3, -4) * (scale<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0)) == point<2>(1.0, 8.0));
    ASSERT_TRUE(translate<2>(-3, -4) * (scale<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0)) == point<3>(1, 8.0, 24.0));
    ASSERT_TRUE(translate<3>(-3, -4, -5) * (scale<2>(4.0, 6.0) * point<2>(1.0, 2.0)) == point<3>(1.0, 8.0, -5));
    ASSERT_TRUE(translate<3>(-3, -4, -5) * (scale<2>(4.0, 6.0) * point<3>(1.0, 2.0, 3.0)) == point<3>(1.0, 8.0, -2.0));
    ASSERT_TRUE(translate<3>(-3, -4, -5) * (scale<3>(4.0, 6.0, 8.0) * point<2>(1.0, 2.0)) == point<3>(1.0, 8.0, -5));
    ASSERT_TRUE(translate<3>(-3, -4, -5) * (scale<3>(4.0, 6.0, 8.0) * point<3>(1.0, 2.0, 3.0)) == point<3>(1, 8.0, 19.0));

    static_assert(std::is_same_v<decltype((translate<2>(-3, -4) * scale<2>(4.0, 6.0)) * point<2>(1.0, 2.0)), point<2>>);
    static_assert(std::is_same_v<decltype((translate<2>(-3, -4) * scale<2>(4.0, 6.0)) * point<3>(1.0, 2.0, 3.0)), point<3>>);
    static_assert(std::is_same_v<decltype((translate<2>(-3, -4) * scale<3>(4.0, 6.0, 8.0)) * point<2>(1.0, 2.0)), point<3>>);
    static_assert(std::is_same_v<decltype((translate<2>(-3, -4) * scale<3>(4.0, 6.0, 8.0)) * point<3>(1.0, 2.0, 3.0)), point<3>>);
    static_assert(std::is_same_v<decltype((translate<3>(-3, -4, -5) * scale<2>(4.0, 6.0)) * point<2>(1.0, 2.0)), point<3>>);
    static_assert(std::is_same_v<decltype((translate<3>(-3, -4, -5) * scale<2>(4.0, 6.0)) * point<3>(1.0, 2.0, 3.0)), point<3>>);
    static_assert(std::is_same_v<decltype((translate<3>(-3, -4, -5) * scale<3>(4.0, 6.0, 8.0)) * point<2>(1.0, 2.0)), point<3>>);
    static_assert(
        std::is_same_v<decltype((translate<3>(-3, -4, -5) * scale<3>(4.0, 6.0, 8.0)) * point<3>(1.0, 2.0, 3.0)), point<3>>);

    static_assert((translate<2>(-3, -4) * scale<2>(4.0, 6.0)) * point<3>(1.0, 2.0, 3.0) == point<3>(1.0, 8.0, 3.0));
    static_assert((translate<2>(-3, -4) * scale<3>(4.0, 6.0, 8.0)) * point<2>(1.0, 2.0) == point<3>(1.0, 8.0, 0.0));
    static_assert((translate<2>(-3, -4) * scale<3>(4.0, 6.0, 8.0)) * point<3>(1.0, 2.0, 3.0) == point<3>(1, 8.0, 24.0));
    static_assert((translate<2>(-3, -4) * scale<2>(4.0, 6.0)) * point<2>(1.0, 2.0) == point<3>(1.0, 8.0, 0.0));
    static_assert((translate<3>(-3, -4, -5) * scale<2>(4.0, 6.0)) * point<2>(1.0, 2.0) == point<3>(1.0, 8.0, -5));
    static_assert((translate<3>(-3, -4, -5) * scale<2>(4.0, 6.0)) * point<3>(1.0, 2.0, 3.0) == point<3>(1.0, 8.0, -2.0));
    static_assert((translate<3>(-3, -4, -5) * scale<3>(4.0, 6.0, 8.0)) * point<2>(1.0, 2.0) == point<3>(1.0, 8.0, -5));
    static_assert((translate<3>(-3, -4, -5) * scale<3>(4.0, 6.0, 8.0)) * point<3>(1.0, 2.0, 3.0) == point<3>(1, 8.0, 19.0));

    ASSERT_TRUE((translate<2>(-3, -4) * scale<2>(4.0, 6.0)) * point<3>(1.0, 2.0, 3.0) == point<3>(1.0, 8.0, 3.0));
    ASSERT_TRUE((translate<2>(-3, -4) * scale<3>(4.0, 6.0, 8.0)) * point<2>(1.0, 2.0) == point<3>(1.0, 8.0, 0.0));
    ASSERT_TRUE((translate<2>(-3, -4) * scale<3>(4.0, 6.0, 8.0)) * point<3>(1.0, 2.0, 3.0) == point<3>(1, 8.0, 24.0));
    ASSERT_TRUE((translate<2>(-3, -4) * scale<2>(4.0, 6.0)) * point<2>(1.0, 2.0) == point<3>(1.0, 8.0, 0.0));
    ASSERT_TRUE((translate<3>(-3, -4, -5) * scale<2>(4.0, 6.0)) * point<2>(1.0, 2.0) == point<3>(1.0, 8.0, -5));
    ASSERT_TRUE((translate<3>(-3, -4, -5) * scale<2>(4.0, 6.0)) * point<3>(1.0, 2.0, 3.0) == point<3>(1.0, 8.0, -2.0));
    ASSERT_TRUE((translate<3>(-3, -4, -5) * scale<3>(4.0, 6.0, 8.0)) * point<2>(1.0, 2.0) == point<3>(1.0, 8.0, -5));
    ASSERT_TRUE((translate<3>(-3, -4, -5) * scale<3>(4.0, 6.0, 8.0)) * point<3>(1.0, 2.0, 3.0) == point<3>(1, 8.0, 19.0));
}
