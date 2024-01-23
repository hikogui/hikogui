// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "translate3.hpp"
#include "transform.hpp"
#include <hikotest/hikotest.hpp>


using namespace hi;

TEST_SUITE(translate3)
{

TEST_CASE(translate_vector)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * vector3(1.0, 2.0, 3.0)), vector3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * vector2(1.0, 2.0)), vector2>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * vector3(1.0, 2.0, 3.0)), vector3>);

    REQUIRE(translate2(4.0, 6.0) * vector3(1.0, 2.0, 3.0) == vector3(1.0, 2.0, 3.0));
    REQUIRE(translate3(4.0, 6.0, 8.0) * vector2(1.0, 2.0) == vector2(1.0, 2.0));
    REQUIRE(translate3(4.0, 6.0, 8.0) * vector3(1.0, 2.0, 3.0) == vector3(1.0, 2.0, 3.0));
    return {};
}

TEST_CASE(translate_point)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * point3(1.0, 2.0, 3.0)), point3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * point2(1.0, 2.0)), point3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0)), point3>);

    REQUIRE(translate2(4.0, 6.0) * point3(1.0, 2.0, 3.0) == point3(5.0, 8.0, 3.0));
    REQUIRE(translate3(4.0, 6.0, 8.0) * point2(1.0, 2.0) == point3(5.0, 8.0, 8.0));
    REQUIRE(translate3(4.0, 6.0, 8.0) * point3(1.0, 2.0, 3.0) == point3(5.0, 8.0, 11.0));
    return {};
}

TEST_CASE(translate_translate)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * translate3(1.0, 2.0, 3.0)), translate3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * translate2(1.0, 2.0)), translate3>);
    static_assert(std::is_same_v<decltype(translate3(4.0, 6.0, 8.0) * translate3(1.0, 2.0, 3.0)), translate3>);

    REQUIRE(translate2(4.0, 6.0) * translate3(1.0, 2.0, 3.0) == translate3(5.0, 8.0, 3.0));
    REQUIRE(translate3(4.0, 6.0, 8.0) * translate2(1.0, 2.0) == translate3(5.0, 8.0, 8.0));
    REQUIRE(translate3(4.0, 6.0, 8.0) * translate3(1.0, 2.0, 3.0) == translate3(5.0, 8.0, 11.0));
    return {};
}

};
