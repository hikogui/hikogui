// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "vector2.hpp"
#include <hikotest/hikotest.hpp>


using namespace hi;

TEST_SUITE(vector2_suite)
{

TEST_CASE(compare_test)
{
    REQUIRE(not (vector2(1.0, 2.0) == vector2(3.0, 4.0)));
    REQUIRE(not (vector2(1.0, 2.0) == vector2(1.0, 4.0)));
    REQUIRE(not (vector2(1.0, 2.0) == vector2(3.0, 2.0)));
    REQUIRE((vector2(1.0, 2.0) == vector2(1.0, 2.0)));

    REQUIRE((vector2(1.0, 2.0) != vector2(3.0, 4.0)));
    REQUIRE((vector2(1.0, 2.0) != vector2(1.0, 4.0)));
    REQUIRE((vector2(1.0, 2.0) != vector2(3.0, 2.0)));
    REQUIRE(not (vector2(1.0, 2.0) != vector2(1.0, 2.0)));
}

TEST_CASE(adding_test)
{
    REQUIRE(vector2(1.0, 2.0) + vector2(3.0, 4.0) == vector2(4.0, 6.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + vector2(3.0, 4.0)), vector2>);
}

TEST_CASE(subtracting_test)
{
    REQUIRE(vector2(1.0, 2.0) - vector2(3.0, 4.0) == vector2(-2.0, -2.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) - vector2(3.0, 4.0)), vector2>);
}

TEST_CASE(scaling_test)
{
    REQUIRE(vector2(1.0, 2.0) * 42.0 == vector2(42.0, 84.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) * 42.0), vector2>);
}

TEST_CASE(invert_test)
{
    REQUIRE(-vector2(1.0, 2.0) == vector2(-1.0, -2.0));

    static_assert(std::is_same_v<decltype(-vector2(1.0, 2.0)), vector2>);
}

TEST_CASE(hypot_test)
{
    REQUIRE(hypot(vector2(1.0, 2.0)) == 2.236067, 0.00001);
}

TEST_CASE(rcp_hypot_test)
{
    REQUIRE(rcp_hypot(vector2(1.0, 2.0)) == 0.447213, 0.0001);
}

TEST_CASE(rcp_normalize_test)
{
    REQUIRE(hypot(normalize(vector2(1.0, 2.0))) == 1.0, 0.001);

    static_assert(std::is_same_v<decltype(normalize(vector2(1.0, 2.0))), vector2>);
}

TEST_CASE(dot_test)
{
    REQUIRE(dot(vector2(1.0, 2.0), vector2(3.0, 4.0)) == 11.0);
}

TEST_CASE(cross_test)
{
    REQUIRE(cross(vector2(4.0, 9.0)) == vector2(-9.0, 4.0));
    REQUIRE(cross(vector2(4.0, 9.0), vector2(4.0, 9.0)) == 0.0);
    REQUIRE(cross(vector2(4.0, 9.0), vector2(-9.0, 4.0)) == 97.0);
}

};
