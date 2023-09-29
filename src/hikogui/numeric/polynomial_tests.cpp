// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "polynomial_tests.hpp"
#include "polynomial.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(polynomial, solve_depressed_cubic)
{
    ASSERT_RESULTS(solveDepressedCubic(6.0, -20.0), make_lean_vector<double>(2.0));
}

TEST(polynomial, solve_cubic)
{
    ASSERT_RESULTS(solvePolynomial(1.0, -6.0, 14.0, -15.0), make_lean_vector<double>(3.0));
    ASSERT_RESULTS(solvePolynomial(1.0, -3.0, 3.0, -1.0), make_lean_vector<double>(1.0));
    ASSERT_RESULTS(solvePolynomial(1.0, 1.0, 1.0, -3.0), make_lean_vector<double>(1.0));
    ASSERT_RESULTS(solvePolynomial(1.0, -5.0, -2.0, 24.0), make_lean_vector<double>(-2.0, 3.0, 4.0));
    ASSERT_RESULTS(solvePolynomial(1.0, -6.0, 11.0, -6.0), make_lean_vector<double>(1.0, 2.0, 3.0));
    ASSERT_RESULTS(solvePolynomial(1.0, 0.0, -7.0, -6.0), make_lean_vector<double>(-2.0, -1.0, 3.0));
    ASSERT_RESULTS(solvePolynomial(1.0, -4.0, -9.0, 36.0), make_lean_vector<double>(-3.0, 3.0, 4.0));
    ASSERT_RESULTS(solvePolynomial(1.0, -6.0, -6.0, -7.0), make_lean_vector<double>(7.0));
    ASSERT_RESULTS(solvePolynomial(1.0, 3.0, 3.0, 1.0), make_lean_vector<double>(-1.0));
    ASSERT_RESULTS(solvePolynomial(1.0, 3.0, -6.0, -8.0), make_lean_vector<double>(2.0, -1.0, -4.0));
    ASSERT_RESULTS(solvePolynomial(1.0, 2.0, -21.0, 18.0), make_lean_vector<double>(3.0, -6.0, 1.0));
    ASSERT_RESULTS(solvePolynomial(1.0, 4.0, 7.0, 6.0), make_lean_vector<double>(-2.0));
    ASSERT_RESULTS(solvePolynomial(2.0, 9.0, 3.0, -4.0), make_lean_vector<double>(-4.0, -1.0, 0.5));

    // Fails because of numeric inaccuracies, solveCubic will return only one real root.
    // ASSERT_RESULTS(solveCubic(1.0, -5.0, 8.0, -4.0), std::make_tuple(1.0, 2.0, 2.0));
}

TEST(polynomial, solve_quadratic)
{
    ASSERT_RESULTS(solvePolynomial(1.0, -10.0, 16.0), make_lean_vector<double>(2.0, 8.0));
    ASSERT_RESULTS(solvePolynomial(18.0, -3.0, -6.0), make_lean_vector<double>(2.0f / 3.0f, -0.5));
    ASSERT_RESULTS(solvePolynomial(50.0, 0.0, -72.0), make_lean_vector<double>(-6.0f / 5.0f, 6.0f / 5.0f));
    ASSERT_RESULTS(solvePolynomial(2.0, -1.0, -3.0), make_lean_vector<double>(3.0 / 2.0, -1.0));
    ASSERT_RESULTS(solvePolynomial(1.0, -2.0, -8.0), make_lean_vector<double>(-2.0, 4.0));
    ASSERT_RESULTS(solvePolynomial(1.0, -2.0, -3.0), make_lean_vector<double>(-1.0, 3.0));
}

TEST(polynomial, solve_linear)
{
    ASSERT_RESULTS(solvePolynomial(2.0, -6.0), make_lean_vector<double>(3.0));
    ASSERT_RESULTS(solvePolynomial(3.0, 6.0), make_lean_vector<double>(-2.0));
}
