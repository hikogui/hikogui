// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "color_space.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace hi;

#define ASSERT_NEAR_VEC(lhs, rhs, abs_err) ASSERT_TRUE(hypot<0b1111>(lhs - rhs) < abs_err)

TEST(color_space, color_primaries_to_RGBtoXYZ)
{
    auto BT709_to_XYZ = color_primaries_to_RGBtoXYZ(0.3127f, 0.3290f, 0.64f, 0.33f, 0.30f, 0.60f, 0.15f, 0.06f);

    ASSERT_NEAR_VEC(get<0>(BT709_to_XYZ), f32x4(0.4124f, 0.2126f, 0.0193f), 0.001);
    ASSERT_NEAR_VEC(get<1>(BT709_to_XYZ), f32x4(0.3576f, 0.7152f, 0.1192f), 0.001);
    ASSERT_NEAR_VEC(get<2>(BT709_to_XYZ), f32x4(0.1805f, 0.0722f, 0.9505f), 0.001);
    ASSERT_NEAR_VEC(get<3>(BT709_to_XYZ), f32x4(0.0f, 0.0f, 0.0f, 1.0f), 0.001);
}
