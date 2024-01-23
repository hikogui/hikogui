// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "vector2.hpp"
#include <hikotest/hikotest.hpp>


using namespace hi;

TEST_SUITE(vector2)
{

TEST_CASE(compare)
{
    REQUIRE(not (vector2(1.0, 2.0) == vector2(3.0, 4.0)));
    REQUIRE(not (vector2(1.0, 2.0) == vector2(1.0, 4.0)));
    REQUIRE(not (vector2(1.0, 2.0) == vector2(3.0, 2.0)));
    REQUIRE((vector2(1.0, 2.0) == vector2(1.0, 2.0)));

    REQUIRE((vector2(1.0, 2.0) != vector2(3.0, 4.0)));
    REQUIRE((vector2(1.0, 2.0) != vector2(1.0, 4.0)));
    REQUIRE((vector2(1.0, 2.0) != vector2(3.0, 2.0)));
    REQUIRE(not (vector2(1.0, 2.0) != vector2(1.0, 2.0)));
    return {};
}

TEST_CASE(adding)
{
    REQUIRE(vector2(1.0, 2.0) + vector2(3.0, 4.0) == vector2(4.0, 6.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + vector2(3.0, 4.0)), vector2>);
    return {};
}

TEST_CASE(subtracting)
{
    REQUIRE(vector2(1.0, 2.0) - vector2(3.0, 4.0) == vector2(-2.0, -2.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) - vector2(3.0, 4.0)), vector2>);
    return {};
}

TEST_CASE(scaling)
{
    REQUIRE(vector2(1.0, 2.0) * 42.0 == vector2(42.0, 84.0));

    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) * 42.0), vector2>);
    return {};
}

TEST_CASE(invert)
{
    REQUIRE(-vector2(1.0, 2.0) == vector2(-1.0, -2.0));

    static_assert(std::is_same_v<decltype(-vector2(1.0, 2.0)), vector2>);
    return {};
}

TEST_CASE(hypot)
{
    REQUIRE(hypot(vector2(1.0, 2.0)) == 2.236067, 0.00001);
    return {};
}

TEST_CASE(rcp_hypot)
{
    REQUIRE(rcp_hypot(vector2(1.0, 2.0)) == 0.447213, 0.0001);
    return {};
}

TEST_CASE(rcp_normalize)
{
    REQUIRE(hypot(normalize(vector2(1.0, 2.0))) == 1.0, 0.001);

    static_assert(std::is_same_v<decltype(normalize(vector2(1.0, 2.0))), vector2>);
    return {};
}

TEST_CASE(dot)
{
    REQUIRE(dot(vector2(1.0, 2.0), vector2(3.0, 4.0)) == 11.0);
    return {};
}

TEST_CASE(cross)
{
    REQUIRE(cross(vector2(4.0, 9.0)) == vector2(-9.0, 4.0));
    REQUIRE(cross(vector2(4.0, 9.0), vector2(4.0, 9.0)) == 0.0);
    REQUIRE(cross(vector2(4.0, 9.0), vector2(-9.0, 4.0)) == 97.0);
    return {};
}

};
