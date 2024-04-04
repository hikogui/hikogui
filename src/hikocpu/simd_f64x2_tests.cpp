// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "simd_intf.hpp"
#include "../macros.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(simd_f64x2_suite)
{

TEST_CASE(swizzle_test)
{
    auto const tmp = hi::f64x2{2.0, 3.0};

    REQUIRE(tmp.xx() == hi::f64x2(2.0, 2.0));
    REQUIRE(tmp.xy() == hi::f64x2(2.0, 3.0));
    REQUIRE(tmp.x0() == hi::f64x2(2.0, 0.0));
    REQUIRE(tmp.x1() == hi::f64x2(2.0, 1.0));

    REQUIRE(tmp.yx() == hi::f64x2(3.0, 2.0));
    REQUIRE(tmp.yy() == hi::f64x2(3.0, 3.0));
    REQUIRE(tmp.y0() == hi::f64x2(3.0, 0.0));
    REQUIRE(tmp.y1() == hi::f64x2(3.0, 1.0));

    REQUIRE(tmp._0x() == hi::f64x2(0.0, 2.0));
    REQUIRE(tmp._0y() == hi::f64x2(0.0, 3.0));
    REQUIRE(tmp._00() == hi::f64x2(0.0, 0.0));
    REQUIRE(tmp._01() == hi::f64x2(0.0, 1.0));

    REQUIRE(tmp._1x() == hi::f64x2(1.0, 2.0));
    REQUIRE(tmp._1y() == hi::f64x2(1.0, 3.0));
    REQUIRE(tmp._10() == hi::f64x2(1.0, 0.0));
    REQUIRE(tmp._11() == hi::f64x2(1.0, 1.0));
}

};
