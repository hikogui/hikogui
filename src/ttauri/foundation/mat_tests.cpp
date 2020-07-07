// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/foundation/vec.hpp"
#include "ttauri/foundation/mat.hpp"
#include "ttauri/foundation/required.hpp"
#include "ttauri/foundation/math.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;

#define ASSERT_NEAR_VEC(lhs, rhs, abs_err)\
    ASSERT_TRUE(length(lhs - rhs) < abs_err)

TEST(Mat, Translate) {
    ttlet tmp = vec{2.0, 3.0, 4.0, 1.0};

    ttlet M1 = mat::T(vec{1.0, 2.0, 3.0});
    ASSERT_EQ(M1 * tmp, vec(3.0, 5.0, 7.0, 1.0));

    ttlet M2 = mat::T(vec{2.0, 2.0, 2.0});
    ASSERT_EQ(M2 * (M1 * tmp), vec(5.0, 7.0, 9.0, 1.0));

    ttlet M3 = M2 * M1;
    ASSERT_EQ(M3 * tmp, vec(5.0, 7.0, 9.0, 1.0));
}

TEST(Mat, Scale) {
    ttlet tmp = vec{2.0, 3.0, 4.0, 1.0};

    ttlet M1 = mat::S(2.0, 2.0, 2.0);
    ASSERT_EQ(M1 * tmp, vec(4.0, 6.0, 8.0, 1.0));

    ttlet M2 = mat::S(3.0, 3.0, 3.0);
    ASSERT_EQ(M2 * (M1 * tmp), vec(12.0, 18.0, 24.0, 1.0));

    ttlet M3 = M2 * M1;
    ASSERT_EQ(M3 * tmp, vec(12.0, 18.0, 24.0, 1.0));
}

TEST(Mat, TranslateScale) {
    ttlet tmp = vec{2.0, 3.0, 4.0, 1.0};

    {
        ttlet M1 = mat::T(1.0, 2.0, 3.0);
        ASSERT_EQ(M1 * tmp, vec(3.0, 5.0, 7.0, 1.0));

        ttlet M2 = mat::S(2.0, 2.0, 2.0);
        ASSERT_EQ(M2 * (M1 * tmp), vec(6.0, 10.0, 14.0, 1.0));

        ttlet M3 = M2 * M1;
        ASSERT_EQ(M3 * tmp, vec(6.0, 10.0, 14.0, 1.0));
    }

    {
        ttlet M1 = mat::S(2.0, 2.0, 2.0);
        ASSERT_EQ(M1 * tmp, vec(4.0, 6.0, 8.0, 1.0));

        ttlet M2 = mat::T(vec{1.0, 2.0, 3.0});
        ASSERT_EQ(M2 * (M1 * tmp), vec(5.0, 8.0, 11.0, 1.0));

        ttlet M3 = M2 * M1;
        ASSERT_EQ(M3 * tmp, vec(5.0, 8.0, 11.0, 1.0));
    }
}

TEST(Mat, Rotate) {
    ttlet tmp = vec{2.0, 3.0, 4.0, 1.0};

    ttlet M1 = mat::R(0.0);
    ASSERT_EQ(M1 * tmp, vec(2.0, 3.0, 4.0, 1.0));

    // 90 degrees counter clock-wise.
    ttlet M2 = mat::R(pi * 0.5f);
    ASSERT_NEAR_VEC(M2 * tmp, vec(-3.0, 2.0, 4.0, 1.0), 0.001);

    // 180 degrees counter clock-wise.
    ttlet M3 = mat::R(pi);
    ASSERT_NEAR_VEC(M3 * tmp, vec(-2.0, -3.0, 4.0, 1.0), 0.001);

    // 270 degrees counter clock-wise.
    ttlet M4 = mat::R(pi * 1.5f);
    ASSERT_NEAR_VEC(M4 * tmp, vec(3.0, -2.0, 4.0, 1.0), 0.001);
}

TEST(Mat, Invert) {
    auto XYZ_to_sRGB = mat{
         3.24096994f, -1.53738318f, -0.49861076f, 0.0f,
        -0.96924364f,  1.87596750f,  0.04155506f, 0.0f,
         0.05563008f, -0.20397696f,  1.05697151f, 0.0f,
         0.0f       ,  0.0f       ,  0.0f       , 1.0f
    };

    auto sRGB_to_XYZ = ~XYZ_to_sRGB;

    ASSERT_NEAR_VEC(sRGB_to_XYZ.get<0>(), vec(0.41239080, 0.21263901, 0.01933082), 0.001);
    ASSERT_NEAR_VEC(sRGB_to_XYZ.get<1>(), vec(0.35758434, 0.71516868, 0.11919478), 0.001);
    ASSERT_NEAR_VEC(sRGB_to_XYZ.get<2>(), vec(0.18048079, 0.07219232, 0.95053215), 0.001);
    ASSERT_NEAR_VEC(sRGB_to_XYZ.get<3>(), vec(0.0, 0.0, 0.0, 1.0), 0.001);

}

TEST(Mat, Color) {
    auto BT709_to_XYZ = mat::RGBtoXYZ(0.3127f, 0.3290f, 0.64f, 0.33f, 0.30f, 0.60f, 0.15f, 0.06f);

    ASSERT_NEAR_VEC(BT709_to_XYZ.get<0>(), vec(0.4124f, 0.2126f, 0.0193f), 0.001);
    ASSERT_NEAR_VEC(BT709_to_XYZ.get<1>(), vec(0.3576f, 0.7152f, 0.1192f), 0.001);
    ASSERT_NEAR_VEC(BT709_to_XYZ.get<2>(), vec(0.1805f, 0.0722f, 0.9505f), 0.001);
    ASSERT_NEAR_VEC(BT709_to_XYZ.get<3>(), vec(0.0, 0.0, 0.0, 1.0), 0.001);


}