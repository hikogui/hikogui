// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "polynomial.hpp"
#include "utility/module.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

template<typename T, typename U, int N>
double maxAbsDiff(hi::results<T, N> const &lhs, hi::results<U, N> const &rhs)
{
    if (lhs.size() != rhs.size()) {
        return std::numeric_limits<double>::infinity();
    }

    double max_diff = 0.0;
    assert(lhs.size() <= lhs.capacity());
    for (auto i = 0_uz; i != lhs.size(); ++i) {
        // Compare with the closest value in rhs.
        double min_diff = std::numeric_limits<double>::infinity();
        for (auto j = 0_uz; j != rhs.size(); ++j) {
            hi::inplace_min(min_diff, std::abs(lhs[i] - rhs[j]));
        }

        hi::inplace_max(max_diff, min_diff);
    }
    return max_diff;
}

template<typename T, typename U, int N>
testing::AssertionResult ResultsNearPredFormat(
    const char *expr1,
    const char *expr2,
    const char *abs_error_expr,
    hi::results<T, N> val1,
    hi::results<U, N> val2,
    double abs_error)
{
    hilet diff = maxAbsDiff(val1, val2);
    if (diff <= abs_error)
        return testing::AssertionSuccess();

    return testing::AssertionFailure() << "The difference between " << expr1 << " and " << expr2 << " is " << diff
                                       << ", which exceeds " << abs_error_expr << ", where\n"
                                       << expr1 << " evaluates to " << val1 << ",\n"
                                       << expr2 << " evaluates to " << val2 << ", and\n"
                                       << abs_error_expr << " evaluates to " << abs_error << ".";
}

#define ASSERT_RESULTS_NEAR(val1, val2, abs_error) ASSERT_PRED_FORMAT3(ResultsNearPredFormat, val1, val2, abs_error)

#define ASSERT_RESULTS(val1, val2) ASSERT_RESULTS_NEAR(val1, val2, 0.000001)

namespace hi::inline v1 {
using results1 = hi::results<double, 1>;
using results2 = hi::results<double, 2>;
using results3 = hi::results<double, 3>;
} // namespace hi::inline v1
