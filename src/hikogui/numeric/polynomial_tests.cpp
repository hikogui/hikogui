// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "polynomial.hpp"
#include <hikotest/hikotest.hpp>
#include <algorithm>

TEST_SUITE(polynomial) {

    hi::lean_vector<double> sort(hi::lean_vector<double> v)
    {
        std::sort(v.begin(), v.end());
        return v;
    }

TEST_CASE(solve_depressed_cubic)
{
    REQUIRE(sort(hi::solveDepressedCubic(6.0, -20.0)) == hi::make_lean_vector<double>(2.0), 0.000001);
}

TEST_CASE(solve_cubic)
{
    REQUIRE(sort(hi::solvePolynomial(1.0, -6.0, 14.0, -15.0)) == hi::make_lean_vector<double>(3.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, -3.0, 3.0, -1.0)) == hi::make_lean_vector<double>(1.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, 1.0, 1.0, -3.0)) == hi::make_lean_vector<double>(1.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, -5.0, -2.0, 24.0)) == hi::make_lean_vector<double>(-2.0, 3.0, 4.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, -6.0, 11.0, -6.0)) == hi::make_lean_vector<double>(1.0, 2.0, 3.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, 0.0, -7.0, -6.0)) == hi::make_lean_vector<double>(-2.0, -1.0, 3.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, -4.0, -9.0, 36.0)) == hi::make_lean_vector<double>(-3.0, 3.0, 4.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, -6.0, -6.0, -7.0)) == hi::make_lean_vector<double>(7.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, 3.0, 3.0, 1.0)) == hi::make_lean_vector<double>(-1.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, 3.0, -6.0, -8.0)) == hi::make_lean_vector<double>(-4.0, -1.0, 2.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, 2.0, -21.0, 18.0)) == hi::make_lean_vector<double>(-6.0, 1.0, 3.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, 4.0, 7.0, 6.0)) == hi::make_lean_vector<double>(-2.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(2.0, 9.0, 3.0, -4.0)) == hi::make_lean_vector<double>(-4.0, -1.0, 0.5), 0.000001);

    // Fails because of numeric inaccuracies, solveCubic will return only one real root.
    // REQUIRE(solveCubic(1.0, -5.0, 8.0, -4.0) == std::make_tuple(1.0, 2.0, 2.0), 0.000001);
}

TEST_CASE(solve_quadratic)
{
    REQUIRE(sort(hi::solvePolynomial(1.0, -10.0, 16.0)) == hi::make_lean_vector<double>(2.0, 8.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(18.0, -3.0, -6.0)) == hi::make_lean_vector<double>(-0.5, 2.0f / 3.0f), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(50.0, 0.0, -72.0)) == hi::make_lean_vector<double>(-6.0f / 5.0f, 6.0f / 5.0f), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(2.0, -1.0, -3.0)) == hi::make_lean_vector<double>(-1.0, 3.0 / 2.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, -2.0, -8.0)) == hi::make_lean_vector<double>(-2.0, 4.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(1.0, -2.0, -3.0)) == hi::make_lean_vector<double>(-1.0, 3.0), 0.000001);
}

TEST_CASE(solve_linear)
{
    REQUIRE(sort(hi::solvePolynomial(2.0, -6.0)) == hi::make_lean_vector<double>(3.0), 0.000001);
    REQUIRE(sort(hi::solvePolynomial(3.0, 6.0)) == hi::make_lean_vector<double>(-2.0), 0.000001);
}

};
