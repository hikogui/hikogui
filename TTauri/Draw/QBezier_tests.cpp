// Copyright 2019 Pokitec
// All rights reserved.

#include "QBezier.hpp"
#include "TTauri/math_tests.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri::Draw;
TEST(QBezierTests, solveXByY) {
    ASSERT_RESULTS(QBezier({ 1,1 }, { 1.5,1 }, { 2,1 }).solveXByY(1.5), TTauri::results2());
    ASSERT_RESULTS(QBezier({ 2,1 }, { 2,1.5 }, { 2,2 }).solveXByY(1.5), TTauri::results2(2));
    ASSERT_RESULTS(QBezier({ 2,2 }, { 1.5,2 }, { 1,2 }).solveXByY(1.5), TTauri::results2());
    ASSERT_RESULTS(QBezier({ 1,2 }, { 1,1.5 }, { 1,1 }).solveXByY(1.5), TTauri::results2(1));
}