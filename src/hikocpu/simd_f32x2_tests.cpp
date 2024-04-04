// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "simd_intf.hpp"
#include "../macros.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(simd_f32x2_suite)
{

TEST_CASE(swizzle_test)
{
    auto const tmp = hi::f32x2{2.0f, 3.0f};

    REQUIRE(tmp.xx() == hi::f32x2(2.0f, 2.0f));
    REQUIRE(tmp.xy() == hi::f32x2(2.0f, 3.0f));
    REQUIRE(tmp.x0() == hi::f32x2(2.0f, 0.0f));
    REQUIRE(tmp.x1() == hi::f32x2(2.0f, 1.0f));

    REQUIRE(tmp.yx() == hi::f32x2(3.0f, 2.0f));
    REQUIRE(tmp.yy() == hi::f32x2(3.0f, 3.0f));
    REQUIRE(tmp.y0() == hi::f32x2(3.0f, 0.0f));
    REQUIRE(tmp.y1() == hi::f32x2(3.0f, 1.0f));

    REQUIRE(tmp._0x() == hi::f32x2(0.0f, 2.0f));
    REQUIRE(tmp._0y() == hi::f32x2(0.0f, 3.0f));
    REQUIRE(tmp._00() == hi::f32x2(0.0f, 0.0f));
    REQUIRE(tmp._01() == hi::f32x2(0.0f, 1.0f));

    REQUIRE(tmp._1x() == hi::f32x2(1.0f, 2.0f));
    REQUIRE(tmp._1y() == hi::f32x2(1.0f, 3.0f));
    REQUIRE(tmp._10() == hi::f32x2(1.0f, 0.0f));
    REQUIRE(tmp._11() == hi::f32x2(1.0f, 1.0f));
}

};
