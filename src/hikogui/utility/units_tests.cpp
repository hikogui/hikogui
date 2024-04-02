// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "units.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(units) {

TEST_CASE(kilometer_to_meter)
{
    REQUIRE(hi::meters{hi::kilometers{15.0}}.count() == 15'000.0, 0.0000001);
}

TEST_CASE(kilometer_to_decimeter)
{
    REQUIRE(hi::decimeters{hi::kilometers{15.0}}.count() == 150'000.0, 0.0000001);
}

TEST_CASE(inch_to_millimeter)
{
    REQUIRE(hi::millimeters{hi::inches{2.0}}.count() == 50.8, 0.0000001);
}

TEST_CASE(centimeter_to_point)
{
    REQUIRE(hi::points{hi::centimeters{2.0}}.count() == 56.69291338582677, 0.0000001);
}

TEST_CASE(add_centimeter_to_centimeter)
{
    static_assert(hi::centimeters{2.0} + hi::centimeters{3.0} == hi::centimeters{5.0});
    REQUIRE(hi::centimeters{2.0} + hi::centimeters{3.0} == hi::centimeters{5.0});
}

TEST_CASE(add_inch_to_point)
{
    static_assert(hi::inches{2.0} + hi::points{3.0} == hi::points{147.0});
    REQUIRE(hi::inches{2.0} + hi::points{3.0} == hi::points{147.0});
}

TEST_CASE(add_inch_to_dip)
{
    static_assert(hi::inches{2.0} + hi::dips{3.0} == hi::dips{195.0});
    REQUIRE(hi::inches{2.0} + hi::dips{3.0} == hi::dips{195.0});
}

TEST_CASE(compare_inch_to_point)
{
    static_assert(hi::inches{2.0} == hi::points{144.0});
    REQUIRE(hi::inches{2.0} == hi::points{144.0});
}

TEST_CASE(divide_inch_by_point)
{
    static_assert(hi::inches{2.0} / hi::points{1.0} == 144.0);
    REQUIRE(hi::inches{2.0} / hi::points{1.0} == 144.0);
}

};
