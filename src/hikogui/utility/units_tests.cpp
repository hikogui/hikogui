// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "units.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>

TEST(units, kilometer_to_meter)
{
    ASSERT_DOUBLE_EQ(hi::meters{hi::kilometers{15.0}}.count(), 15'000.0);
}

TEST(units, kilometer_to_decimeter)
{
    ASSERT_DOUBLE_EQ(hi::decimeters{hi::kilometers{15.0}}.count(), 150'000.0);
}

TEST(units, inch_to_millimeter)
{
    ASSERT_DOUBLE_EQ(hi::millimeters{hi::inches{2.0}}.count(), 50.8);
}

TEST(units, centimeter_to_point)
{
    ASSERT_DOUBLE_EQ(hi::points{hi::centimeters{2.0}}.count(), 56.69291338582677);
}

TEST(units, add_centimeter_to_centimeter)
{
    static_assert(hi::centimeters{2.0} + hi::centimeters{3.0} == hi::centimeters{5.0});
    ASSERT_EQ(hi::centimeters{2.0} + hi::centimeters{3.0}, hi::centimeters{5.0});
}

TEST(units, add_inch_to_point)
{
    static_assert(hi::inches{2.0} + hi::points{3.0} == hi::points{147.0});
    ASSERT_EQ(hi::inches{2.0} + hi::points{3.0}, hi::points{147.0});
}

TEST(units, add_inch_to_dip)
{
    static_assert(hi::inches{2.0} + hi::dips{3.0} == hi::dips{195.0});
    ASSERT_EQ(hi::inches{2.0} + hi::dips{3.0}, hi::dips{195.0});
}

TEST(units, compare_inch_to_point)
{
    static_assert(hi::inches{2.0} == hi::points{144.0});
    ASSERT_EQ(hi::inches{2.0}, hi::points{144.0});
}

TEST(units, divide_inch_by_point)
{
    static_assert(hi::inches{2.0} / hi::points{1.0} == 144.0);
    ASSERT_EQ(hi::inches{2.0} / hi::points{1.0}, 144.0);
}
