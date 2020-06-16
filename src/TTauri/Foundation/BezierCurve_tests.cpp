// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/BezierCurve.hpp"
#include "TTauri/Foundation/polynomial_tests.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;
TEST(BezierTests, solveXByY) {
    ASSERT_RESULTS(BezierCurve(vec::point( 1,1 ), vec::point( 1.5,1 ), vec::point( 2,1 )).solveXByY(1.5), tt::results3());
    ASSERT_RESULTS(BezierCurve(vec::point( 2,1 ), vec::point( 2,1.5 ), vec::point( 2,2 )).solveXByY(1.5), tt::results3(2));
    ASSERT_RESULTS(BezierCurve(vec::point( 2,2 ), vec::point( 1.5,2 ), vec::point( 1,2 )).solveXByY(1.5), tt::results3());
    ASSERT_RESULTS(BezierCurve(vec::point( 1,2 ), vec::point( 1,1.5 ), vec::point( 1,1 )).solveXByY(1.5), tt::results3(1));
}
