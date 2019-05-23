
// Copyright 2019 Pokitec
// All rights reserved.

#include <TTauri/math.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

template<int N>
testing::AssertionResult ResultsNearPredFormat(const char* expr1,
    const char* expr2,
    const char* abs_error_expr,
    TTauri::results<float, N> val1,
    TTauri::results<float, N> val2,
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