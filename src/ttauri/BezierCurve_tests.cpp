// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/BezierCurve.hpp"
#include "ttauri/polynomial_tests.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;
TEST(BezierTests, solveXByY) {
    ASSERT_RESULTS(BezierCurve(f32x4::point({1.0f,1.0f}), f32x4::point({1.5f,1.0f}), f32x4::point({2.0f,1.0f})).solveXByY(1.5f), tt::results3());
    ASSERT_RESULTS(BezierCurve(f32x4::point({2.0f,1.0f}), f32x4::point({2.0f,1.5f}), f32x4::point({2.0f,2.0f})).solveXByY(1.5f), tt::results3(2.0f));
    ASSERT_RESULTS(BezierCurve(f32x4::point({2.0f,2.0f}), f32x4::point({1.5f,2.0f}), f32x4::point({1.0f,2.0f})).solveXByY(1.5f), tt::results3());
    ASSERT_RESULTS(BezierCurve(f32x4::point({1.0f,2.0f}), f32x4::point({1.0f,1.5f}), f32x4::point({1.0f,1.0f})).solveXByY(1.5f), tt::results3(1.0f));
}
