// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "translate2.hpp"
#include "transform.hpp"
#include <hikotest/hikotest.hpp>

using namespace hi;

TEST_SUITE(translate2_suite)
{

TEST_CASE(translate_vector_test)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * vector2(1.0, 2.0)), vector2>);

    REQUIRE(translate2(4.0, 6.0) * vector2(1.0, 2.0) == vector2(1.0, 2.0));
}

TEST_CASE(translate_point_test)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * point2(1.0, 2.0)), point2>);

    REQUIRE(translate2(4.0, 6.0) * point2(1.0, 2.0) == point2(5.0, 8.0));
}

TEST_CASE(translate_translate_test)
{
    static_assert(std::is_same_v<decltype(translate2(4.0, 6.0) * translate2(1.0, 2.0)), translate2>);

    REQUIRE(translate2(4.0, 6.0) * translate2(1.0, 2.0) == translate2(5.0, 8.0));
}

};
