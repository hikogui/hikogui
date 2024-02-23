// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "transform.hpp"
#include <hikotest/hikotest.hpp>

using namespace hi;

TEST_SUITE(transform_suite)
{

TEST_CASE(translate_scale_point_test)
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

    REQUIRE(translate2(-3, -4) * (scale2(4.0, 6.0) * point2(1.0, 2.0)) == point2(1.0, 8.0));
    REQUIRE(translate2(-3, -4) * (scale2(4.0, 6.0) * point3(1.0, 2.0, 3.0)) == point3(1.0, 8.0, 3.0));
    REQUIRE(translate2(-3, -4) * (scale3(4.0, 6.0, 8.0) * point2(1.0, 2.0)) == point2(1.0, 8.0));
    REQUIRE(translate2(-3, -4) * (scale3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)) == point3(1, 8.0, 24.0));
    REQUIRE(translate3(-3, -4, -5) * (scale2(4.0, 6.0) * point2(1.0, 2.0)) == point3(1.0, 8.0, -5));
    REQUIRE(translate3(-3, -4, -5) * (scale2(4.0, 6.0) * point3(1.0, 2.0, 3.0)) == point3(1.0, 8.0, -2.0));
    REQUIRE(translate3(-3, -4, -5) * (scale3(4.0, 6.0, 8.0) * point2(1.0, 2.0)) == point3(1.0, 8.0, -5));
    REQUIRE(translate3(-3, -4, -5) * (scale3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)) == point3(1, 8.0, 19.0));

    static_assert(std::is_same_v<decltype((translate2(-3, -4) * scale2(4.0, 6.0)) * point2(1.0, 2.0)), point2>);
    static_assert(std::is_same_v<decltype((translate2(-3, -4) * scale2(4.0, 6.0)) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype((translate2(-3, -4) * scale3(4.0, 6.0, 8.0)) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype((translate2(-3, -4) * scale3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype((translate3(-3, -4, -5) * scale2(4.0, 6.0)) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype((translate3(-3, -4, -5) * scale2(4.0, 6.0)) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype((translate3(-3, -4, -5) * scale3(4.0, 6.0, 8.0)) * point2(1.0, 2.0)), point3>);
    static_assert(
        std::is_same_v<decltype((translate3(-3, -4, -5) * scale3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0)), point3>);

    REQUIRE((translate2(-3, -4) * scale2(4.0, 6.0)) * point3(1.0, 2.0, 3.0) == point3(1.0, 8.0, 3.0));
    REQUIRE((translate2(-3, -4) * scale3(4.0, 6.0, 8.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, 0.0));
    REQUIRE((translate2(-3, -4) * scale3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0) == point3(1, 8.0, 24.0));
    REQUIRE((translate2(-3, -4) * scale2(4.0, 6.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, 0.0));
    REQUIRE((translate3(-3, -4, -5) * scale2(4.0, 6.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, -5));
    REQUIRE((translate3(-3, -4, -5) * scale2(4.0, 6.0)) * point3(1.0, 2.0, 3.0) == point3(1.0, 8.0, -2.0));
    REQUIRE((translate3(-3, -4, -5) * scale3(4.0, 6.0, 8.0)) * point2(1.0, 2.0) == point3(1.0, 8.0, -5));
    REQUIRE((translate3(-3, -4, -5) * scale3(4.0, 6.0, 8.0)) * point3(1.0, 2.0, 3.0) == point3(1, 8.0, 19.0));
}

};
