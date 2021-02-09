// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/bezier_curve.hpp"
#include "ttauri/polynomial_tests.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(bezier_cruve, solve_x_by_y) {
    ASSERT_RESULTS(bezier_curve(f32x4::point({1.0f,1.0f}), f32x4::point({1.5f,1.0f}), f32x4::point({2.0f,1.0f})).solveXByY(1.5f), tt::results3());
    ASSERT_RESULTS(bezier_curve(f32x4::point({2.0f,1.0f}), f32x4::point({2.0f,1.5f}), f32x4::point({2.0f,2.0f})).solveXByY(1.5f), tt::results3(2.0f));
    ASSERT_RESULTS(bezier_curve(f32x4::point({2.0f,2.0f}), f32x4::point({1.5f,2.0f}), f32x4::point({1.0f,2.0f})).solveXByY(1.5f), tt::results3());
    ASSERT_RESULTS(bezier_curve(f32x4::point({1.0f,2.0f}), f32x4::point({1.0f,1.5f}), f32x4::point({1.0f,1.0f})).solveXByY(1.5f), tt::results3(1.0f));
}
