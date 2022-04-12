// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/bezier_curve.hpp"
#include "hikogui/polynomial_tests.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(bezier_cruve, solve_x_by_y)
{
    ASSERT_RESULTS(bezier_curve(point2(1.0f, 1.0f), point2(1.5f, 1.0f), point2(2.0f, 1.0f)).solveXByY(1.5f), hi::results3());
    ASSERT_RESULTS(bezier_curve(point2(2.0f, 1.0f), point2(2.0f, 1.5f), point2(2.0f, 2.0f)).solveXByY(1.5f), hi::results3(2.0f));
    ASSERT_RESULTS(bezier_curve(point2(2.0f, 2.0f), point2(1.5f, 2.0f), point2(1.0f, 2.0f)).solveXByY(1.5f), hi::results3());
    ASSERT_RESULTS(bezier_curve(point2(1.0f, 2.0f), point2(1.0f, 1.5f), point2(1.0f, 1.0f)).solveXByY(1.5f), hi::results3(1.0f));
}
