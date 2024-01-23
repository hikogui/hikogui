// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "point2.hpp"
#include <hikotest/hikotest.hpp>

using namespace hi;

TEST_SUITE(point2)
{

TEST_CASE(compare)
{
    REQUIRE(not (point2(1.0, 2.0) == point2(3.0, 4.0)));
    REQUIRE(not (point2(1.0, 2.0) == point2(1.0, 4.0)));
    REQUIRE(not (point2(1.0, 2.0) == point2(3.0, 2.0)));
    REQUIRE((point2(1.0, 2.0) == point2(1.0, 2.0)));

    REQUIRE((point2(1.0, 2.0) != point2(3.0, 4.0)));
    REQUIRE((point2(1.0, 2.0) != point2(1.0, 4.0)));
    REQUIRE((point2(1.0, 2.0) != point2(3.0, 2.0)));
    REQUIRE(not (point2(1.0, 2.0) != point2(1.0, 2.0)));
    return {};
}

TEST_CASE(adding)
{
    REQUIRE(point2(1.0, 2.0) + vector2(3.0, 4.0) == point2(4.0, 6.0));

    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) + vector2(3.0, 4.0)), point2>);

    REQUIRE(vector2(1.0, 2.0) + point2(3.0, 4.0) == point2(4.0, 6.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + point2(3.0, 4.0)), point2>);
    return {};
}

TEST_CASE(subtracting)
{
    REQUIRE(point2(1.0, 2.0) - point2(3.0, 4.0) == vector2(-2.0, -2.0));

    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - point2(3.0, 4.0)), vector2>);

    REQUIRE(point2(1.0, 2.0) - vector2(3.0, 4.0) == point2(-2.0, -2.0));

    static_assert(std::is_same_v<decltype(point2(1.0, 2.0) - vector2(3.0, 4.0)), point2>);
    return {};
}

};
