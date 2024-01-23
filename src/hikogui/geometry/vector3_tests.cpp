// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "vector3.hpp"
#include <hikotest/hikotest.hpp>


using namespace hi;

TEST_SUITE(vector3)
{

TEST_CASE(compare)
{
    REQUIRE(not (vector3(1.0, 2.0, 3.0) == vector3(3.0, 4.0, 5.0)));
    REQUIRE(not (vector3(1.0, 2.0, 3.0) == vector3(1.0, 4.0, 5.0)));
    REQUIRE(not (vector3(1.0, 2.0, 3.0) == vector3(3.0, 2.0, 5.0)));
    REQUIRE((vector3(1.0, 2.0, 3.0) == vector3(1.0, 2.0, 3.0)));

    REQUIRE(not (vector2(1.0, 2.0) == vector3(3.0, 4.0, 5.0)));
    REQUIRE(not (vector2(1.0, 2.0) == vector3(1.0, 4.0, 5.0)));
    REQUIRE(not (vector2(1.0, 2.0) == vector3(3.0, 2.0, 5.0)));
    REQUIRE(not (vector2(1.0, 2.0) == vector3(1.0, 2.0, 3.0)));
    REQUIRE((vector2(1.0, 2.0) == vector3(1.0, 2.0, 0.0)));
    return {};
}

TEST_CASE(adding)
{
    REQUIRE(vector3(1.0, 2.0, 3.0) + vector3(3.0, 4.0, 5.0) == vector3(4.0, 6.0, 8.0));
    REQUIRE(vector2(1.0, 2.0) + vector3(3.0, 4.0, 5.0) == vector3(4.0, 6.0, 5.0));
    REQUIRE(vector3(1.0, 2.0, 3.0) + vector2(3.0, 4.0) == vector3(4.0, 6.0, 3.0));

    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) + vector2(3.0, 4.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) + vector3(3.0, 4.0, 5.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) + vector3(3.0, 4.0, 5.0)), vector3>);
    return {};
}

TEST_CASE(subtracting)
{
    REQUIRE(vector3(1.0, 2.0, 3.0) - vector3(3.0, 4.0, 5.0) == vector3(-2.0, -2.0, -2.0));
    REQUIRE(vector2(1.0, 2.0) - vector3(3.0, 4.0, 5.0) == vector3(-2.0, -2.0, -5.0));
    REQUIRE(vector3(1.0, 2.0, 3.0) - vector2(3.0, 4.0) == vector3(-2.0, -2.0, 3.0));

    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) - vector2(3.0, 4.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector2(1.0, 2.0) - vector3(3.0, 4.0, 5.0)), vector3>);
    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) - vector3(3.0, 4.0, 5.0)), vector3>);
    return {};
}

TEST_CASE(scaling)
{
    REQUIRE(vector3(1.0, 2.0, 3.0) * 42.0 == vector3(42.0, 84.0, 126.0));

    static_assert(std::is_same_v<decltype(vector3(1.0, 2.0, 3.0) * 42.0), vector3>);
    return {};
}

TEST_CASE(invert)
{
    REQUIRE(-vector3(1.0, 2.0, 3.0) == vector3(-1.0, -2.0, -3.0));

    static_assert(std::is_same_v<decltype(-vector3(1.0, 2.0, 3.0)), vector3>);
    return {};
}

TEST_CASE(hypot)
{
    REQUIRE(hypot(vector3(1.0, 2.0, 3.0)) == 3.741657, 0.00001);
    return {};
}

TEST_CASE(rcp_hypot)
{
    REQUIRE(rcp_hypot(vector3(1.0, 2.0, 3.0)) == 0.267261, 0.0001);
    return {};
}

TEST_CASE(rcp_normalize)
{
    REQUIRE(hypot(normalize(vector3(1.0, 2.0, 3.0))) == 1.0, 0.001);

    static_assert(std::is_same_v<decltype(normalize(vector3(1.0, 2.0, 3.0))), vector3>);
    return {};
}

TEST_CASE(dot)
{
    REQUIRE(dot(vector2(1.0, 2.0), vector3(3.0, 4.0, 5.0)) == 11.0);
    REQUIRE(dot(vector3(1.0, 2.0, 3.0), vector2(3.0, 4.0)) == 11.0);
    REQUIRE(dot(vector3(1.0, 2.0, 3.0), vector3(3.0, 4.0, 5.0)) == 26.0);
    return {};
}

TEST_CASE(cross)
{
    REQUIRE(cross(vector3(2.0f, 3.0f, 4.0f), vector3(5.0f, 6.0f, 7.0f)) == vector3(-3.0f, 6.0f, -3.0f));
    REQUIRE(cross(vector3(3.0, -3.0, 1.0), vector3(4.0, 9.0, 2.0)) == vector3(-15.0, -2.0, 39.0));
    return {};
}

};
