// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "scale2.hpp"
#include "transform.hpp"
#include <hikotest/hikotest.hpp>

using namespace hi;

TEST_SUITE(scale2)
{

TEST_CASE(scale_vector)
{
    static_assert(std::is_same_v<decltype(scale2(4.0, 6.0) * vector2(1.0, 2.0)), vector2>);

    REQUIRE(scale2(4.0, 6.0) * vector2(1.0, 2.0) == vector2(4.0, 12.0));
    return {};
}

TEST_CASE(scale_point)
{
    static_assert(std::is_same_v<decltype(scale2(4.0, 6.0) * point2(1.0, 2.0)), point2>);

    REQUIRE(scale2(4.0, 6.0) * point2(1.0, 2.0) == point2(4.0, 12.0));
    return {};
}

TEST_CASE(scale_scale)
{
    static_assert(std::is_same_v<decltype(scale2(4.0, 6.0) * scale2(1.0, 2.0)), scale2>);

    REQUIRE(scale2(4.0, 6.0) * scale2(1.0, 2.0) == scale2(4.0, 12.0));
    return {};
}

};
