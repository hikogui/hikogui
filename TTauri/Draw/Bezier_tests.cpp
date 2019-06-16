// Copyright 2019 Pokitec
// All rights reserved.

#include "Bezier.hpp"
#include "TTauri/math_tests.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri::Draw;
TEST(BezierTests, solveXByY) {
    ASSERT_RESULTS(Bezier({ 1,1 }, { 1.5,1 }, { 2,1 }).solveXByY(1.5), TTauri::results3());
    ASSERT_RESULTS(Bezier({ 2,1 }, { 2,1.5 }, { 2,2 }).solveXByY(1.5), TTauri::results3(2));
    ASSERT_RESULTS(Bezier({ 2,2 }, { 1.5,2 }, { 1,2 }).solveXByY(1.5), TTauri::results3());
    ASSERT_RESULTS(Bezier({ 1,2 }, { 1,1.5 }, { 1,1 }).solveXByY(1.5), TTauri::results3(1));
}