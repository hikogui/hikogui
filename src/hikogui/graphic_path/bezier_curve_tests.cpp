// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "bezier_curve.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(bezier_curve) {

TEST_CASE(solve_x_by_y)
{
    REQUIRE(hi::bezier_curve(hi::point2(1.0f, 1.0f), hi::point2(1.5f, 1.0f), hi::point2(2.0f, 1.0f)).solveXByY(1.5f) == hi::make_lean_vector<double>(), 0.000001);
    REQUIRE(hi::bezier_curve(hi::point2(2.0f, 1.0f), hi::point2(2.0f, 1.5f), hi::point2(2.0f, 2.0f)).solveXByY(1.5f) == hi::make_lean_vector<double>(2.0f), 0.000001);
    REQUIRE(hi::bezier_curve(hi::point2(2.0f, 2.0f), hi::point2(1.5f, 2.0f), hi::point2(1.0f, 2.0f)).solveXByY(1.5f) == hi::make_lean_vector<double>(), 0.000001);
    REQUIRE(hi::bezier_curve(hi::point2(1.0f, 2.0f), hi::point2(1.0f, 1.5f), hi::point2(1.0f, 1.0f)).solveXByY(1.5f) == hi::make_lean_vector<double>(1.0f), 0.000001);
}

};
