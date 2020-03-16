// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/mat.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/math.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace TTauri;

#define ASSERT_NEAR_VEC(lhs, rhs, abs_err)\
    ASSERT_TRUE(length(lhs - rhs) < abs_err)

TEST(Mat, Translate) {
    let tmp = vec{2.0, 3.0, 4.0, 1.0};

    let M1 = mat::T({1.0, 2.0, 3.0});
    ASSERT_EQ(M1 * tmp, vec(3.0, 5.0, 7.0, 1.0));

    let M2 = mat::T({2.0, 2.0, 2.0});
    ASSERT_EQ(M2 * (M1 * tmp), vec(5.0, 7.0, 9.0, 1.0));

    let M3 = M2 * M1;
    ASSERT_EQ(M3 * tmp, vec(5.0, 7.0, 9.0, 1.0));
}

TEST(Mat, Scale) {
    let tmp = vec{2.0, 3.0, 4.0, 1.0};

    let M1 = mat::S(2.0);
    ASSERT_EQ(M1 * tmp, vec(4.0, 6.0, 8.0, 1.0));

    let M2 = mat::S(3.0);
    ASSERT_EQ(M2 * (M1 * tmp), vec(12.0, 18.0, 24.0, 1.0));

    let M3 = M2 * M1;
    ASSERT_EQ(M3 * tmp, vec(12.0, 18.0, 24.0, 1.0));
}

TEST(Mat, TranslateScale) {
    let tmp = vec{2.0, 3.0, 4.0, 1.0};

    {
        let M1 = mat::T({1.0, 2.0, 3.0});
        ASSERT_EQ(M1 * tmp, vec(3.0, 5.0, 7.0, 1.0));

        let M2 = mat::S(2.0);
        ASSERT_EQ(M2 * (M1 * tmp), vec(6.0, 10.0, 14.0, 1.0));

        let M3 = M2 * M1;
        ASSERT_EQ(M3 * tmp, vec(6.0, 10.0, 14.0, 1.0));
    }

    {
        let M1 = mat::S(2.0);
        ASSERT_EQ(M1 * tmp, vec(4.0, 6.0, 8.0, 1.0));

        let M2 = mat::T({1.0, 2.0, 3.0});
        ASSERT_EQ(M2 * (M1 * tmp), vec(5.0, 8.0, 11.0, 1.0));

        let M3 = M2 * M1;
        ASSERT_EQ(M3 * tmp, vec(5.0, 8.0, 11.0, 1.0));
    }
}

TEST(Mat, Rotate) {
    let tmp = vec{2.0, 3.0, 4.0, 1.0};

    let M1 = mat::R(0.0);
    ASSERT_EQ(M1 * tmp, vec(2.0, 3.0, 4.0, 1.0));

    // 90 degrees counter clock-wise.
    let M2 = mat::R(pi * 0.5f);
    ASSERT_NEAR_VEC(M2 * tmp, vec(-3.0, 2.0, 4.0, 1.0), 0.001);

    // 180 degrees counter clock-wise.
    let M3 = mat::R(pi);
    ASSERT_NEAR_VEC(M3 * tmp, vec(-2.0, -3.0, 4.0, 1.0), 0.001);

    // 270 degrees counter clock-wise.
    let M4 = mat::R(pi * 1.5f);
    ASSERT_NEAR_VEC(M4 * tmp, vec(3.0, -2.0, 4.0, 1.0), 0.001);
}