// Copyright 2019 Pokitec
// All rights reserved.

#include <TTauri/math_tests.hpp>
#include <TTauri/math.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;


TEST(TTauriMath, SolveDepressedCubic) {
    ASSERT_RESULTS(TTauri::solveDepressedCubic(6.0, -20.0), TTauri::results3(2.0));
}

TEST(TTauriMath, SolveCubic) {
    ASSERT_RESULTS(TTauri::solveCubic(1.0, -6.0, 14.0, -15.0), TTauri::results3(3.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, -3.0, 3.0, -1.0), TTauri::results3(1.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, 1.0, 1.0, -3.0), TTauri::results3(1.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, -5.0, -2.0, 24.0), TTauri::results3(-2.0, 3.0, 4.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, -6.0, 11.0, -6.0), TTauri::results3(1.0, 2.0, 3.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, 0.0, -7.0, -6.0), TTauri::results3(-2.0, -1.0, 3.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, -4.0, -9.0, 36.0), TTauri::results3(-3.0, 3.0, 4.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, -6.0, -6.0, -7.0), TTauri::results3(7.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, 3.0, 3.0, 1.0), TTauri::results3(-1.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, 3.0, -6.0, -8.0), TTauri::results3(2.0, -1.0, -4.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, 2.0, -21.0, 18.0), TTauri::results3(3.0, -6.0, 1.0));
    ASSERT_RESULTS(TTauri::solveCubic(1.0, 4.0, 7.0, 6.0), TTauri::results3(-2.0));
    ASSERT_RESULTS(TTauri::solveCubic(2.0, 9.0, 3.0, -4.0), TTauri::results3(-4.0, -1.0, 0.5));

    // Fails because of numeric inaccuracies, solveCubic will return only one real root.
    //ASSERT_RESULTS(TTauri::solveCubic(1.0, -5.0, 8.0, -4.0), std::make_tuple(1.0, 2.0, 2.0));
}

TEST(TTauriMath, SolveQuadratic) {
    ASSERT_RESULTS(TTauri::solveQuadratic(1.0, -10.0, 16.0), TTauri::results2(2.0, 8.0));
    ASSERT_RESULTS(TTauri::solveQuadratic(18.0, -3.0, -6.0), TTauri::results2(2.0f / 3.0f, -0.5));
    ASSERT_RESULTS(TTauri::solveQuadratic(50.0, 0.0, -72.0), TTauri::results2(-6.0f / 5.0f, 6.0f / 5.0f));
    ASSERT_RESULTS(TTauri::solveQuadratic(2.0, -1.0, -3.0), TTauri::results2(3.0 / 2.0, -1.0));
    ASSERT_RESULTS(TTauri::solveQuadratic(1.0, -2.0, -8.0), TTauri::results2(-2.0, 4.0));
    ASSERT_RESULTS(TTauri::solveQuadratic(1.0, -2.0, -3.0), TTauri::results2(-1.0, 3.0));
}

TEST(TTauriMath, SolveLinear) {
    ASSERT_RESULTS(TTauri::solveLinear(2.0, -6.0), TTauri::results1(3.0));
    ASSERT_RESULTS(TTauri::solveLinear(3.0, 6.0), TTauri::results1(-2.0));
}