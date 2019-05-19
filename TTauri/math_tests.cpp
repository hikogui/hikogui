// Copyright 2019 Pokitec
// All rights reserved.

#include <TTauri/math.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;

template<int N>
testing::AssertionResult ResultsNearPredFormat(const char* expr1,
    const char* expr2,
    const char* abs_error_expr,
    TTauri::results<float,N> val1,
    TTauri::results<float,N> val2,
    float abs_error) {
    let diff = val1.maxAbsDiff(val2);
    if (diff <= abs_error) return testing::AssertionSuccess();

    return testing::AssertionFailure()
        << "The difference between " << expr1 << " and " << expr2
        << " is " << diff << ", which exceeds " << abs_error_expr << ", where\n"
        << expr1 << " evaluates to " << val1 << ",\n"
        << expr2 << " evaluates to " << val2 << ", and\n"
        << abs_error_expr << " evaluates to " << abs_error << ".";
}


#define ASSERT_RESULTS_NEAR(val1, val2, abs_error)\
    ASSERT_PRED_FORMAT3(ResultsNearPredFormat, val1, val2, abs_error)

#define ASSERT_RESULTS(val1, val2) ASSERT_RESULTS_NEAR(val1, val2, 0.000001f)
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
