// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "point3.hpp"
#include "point2.hpp"
#include <hikotest/hikotest.hpp>

using namespace hi;

TEST_SUITE(point3_suite)
{

TEST_CASE(compare_test)
{
    REQUIRE(not (point3(1.0, 2.0, 3.0) == point3(3.0, 4.0, 5.0)));
    REQUIRE(not (point3(1.0, 2.0, 3.0) == point3(1.0, 4.0, 5.0)));
    REQUIRE(not (point3(1.0, 2.0, 3.0) == point3(3.0, 2.0, 5.0)));
    REQUIRE((point3(1.0, 2.0, 3.0) == point3(1.0, 2.0, 3.0)));

    REQUIRE(not (point2(1.0, 2.0) == point3(3.0, 4.0, 5.0)));
    REQUIRE(not (point2(1.0, 2.0) == point3(1.0, 4.0, 5.0)));
    REQUIRE(not (point2(1.0, 2.0) == point3(3.0, 2.0, 5.0)));
    REQUIRE(not (point2(1.0, 2.0) == point3(1.0, 2.0, 3.0)));
    REQUIRE((point2(1.0, 2.0) == point3(1.0, 2.0, 0.0)));
}

TEST_CASE(adding_test)
{
    REQUIRE(point3(1.0, 2.0, 3.0) + vector3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 8.0));
    REQUIRE(point2(1.0, 2.0) + vector3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 5.0));
    REQUIRE(point3(1.0, 2.0, 3.0) + vector2(3.0, 4.0) == point3(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) + vector2(3.0, 4.0)), point3>);
    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) + vector3(3.0, 4.0, 5.0)), point3>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) + vector3(3.0, 4.0, 5.0)), point3>);

    REQUIRE(vector3(1.0, 2.0, 3.0) + point3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 8.0));
    REQUIRE(vector2(1.0, 2.0) + point3(3.0, 4.0, 5.0) == point3(4.0, 6.0, 5.0));
    REQUIRE(vector3(1.0, 2.0, 3.0) + point2(3.0, 4.0) == point3(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) + point2(3.0, 4.0)), point3>);
    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + point3(3.0, 4.0, 5.0)), point3>);
    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) + point3(3.0, 4.0, 5.0)), point3>);
}

TEST_CASE(subtracting_test)
{
    REQUIRE(point3(1.0, 2.0, 3.0) - point3(3.0, 4.0, 5.0) == vector3(-2.0, -2.0, -2.0));
    REQUIRE(point2(1.0, 2.0) - point3(3.0, 4.0, 5.0) == vector3(-2.0, -2.0, -5.0));
    REQUIRE(point3(1.0, 2.0, 3.0) - point2(3.0, 4.0) == vector3(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - point2(3.0, 4.0)), vector3>);
    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - point3(3.0, 4.0, 5.0)), vector3>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - point3(3.0, 4.0, 5.0)), vector3>);

    REQUIRE(point3(1.0, 2.0, 3.0) - vector3(3.0, 4.0, 5.0) == point3(-2.0, -2.0, -2.0));
    REQUIRE(point2(1.0, 2.0) - vector3(3.0, 4.0, 5.0) == point3(-2.0, -2.0, -5.0));
    REQUIRE(point3(1.0, 2.0, 3.0) - vector2(3.0, 4.0) == point3(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - vector2(3.0, 4.0)), point3>);
    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - vector3(3.0, 4.0, 5.0)), point3>);
    static_assert(std::is_same_v<decltype(point3(1.0, 2.0, 3.0) - vector3(3.0, 4.0, 5.0)), point3>);
}

};
