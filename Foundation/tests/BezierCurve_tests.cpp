// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/BezierCurve.hpp"
#include "TTauri/Foundation/polynomial_tests.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;
TEST(BezierTests, solveXByY) {
    ASSERT_RESULTS(BezierCurve({ 1,1 }, { 1.5,1 }, { 2,1 }).solveXByY(1.5), TTauri::results3());
    ASSERT_RESULTS(BezierCurve({ 2,1 }, { 2,1.5 }, { 2,2 }).solveXByY(1.5), TTauri::results3(2));
    ASSERT_RESULTS(BezierCurve({ 2,2 }, { 1.5,2 }, { 1,2 }).solveXByY(1.5), TTauri::results3());
    ASSERT_RESULTS(BezierCurve({ 1,2 }, { 1,1.5 }, { 1,1 }).solveXByY(1.5), TTauri::results3(1));
}
