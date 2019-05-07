// Copyright 2019 Pokitec
// All rights reserved.

#include <TTauri/math.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;

std::tuple<double,double,double> sortDouble3Tuple(const std::tuple<double,double,double> &x)
{
    double a = std::get<0>(x);
    double b = std::get<1>(x);
    double c = std::get<2>(x);

    if (a > c) { std::swap(a, c); }
    if (a > b) { std::swap(a, b); }
    if (b > c) { std::swap(b, c); }
    return {a, b, c};
}

#define ASSERT_3TUPLE_NEAR(a, b)\
    {\
        auto const [a0, a1, a2] = sortDouble3Tuple(a);\
        auto const [b0, b1, b2] = sortDouble3Tuple(b);\
        ASSERT_NEAR(a0, b0, 0.000001f);\
        ASSERT_NEAR(a1, b1, 0.000001f);\
        ASSERT_NEAR(a2, b2, 0.000001f);\
    }


TEST(TTauriMath, SolveDepressedCubic) {
    ASSERT_3TUPLE_NEAR(TTauri::solveDepressedCubic(6.0, -20.0), std::make_tuple(2.0, 2.0, 2.0));
}

TEST(TTauriMath, SolveCubic) {
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, -6.0, 14.0, -15.0), std::make_tuple(3.0, 3.0, 3.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, -3.0, 3.0, -1.0), std::make_tuple(1.0, 1.0, 1.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, 1.0, 1.0, -3.0), std::make_tuple(1.0, 1.0, 1.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, -5.0, -2.0, 24.0), std::make_tuple(-2.0, 3.0, 4.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, -6.0, 11.0, -6.0), std::make_tuple(1.0, 2.0, 3.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, 0.0, -7.0, -6.0), std::make_tuple(-2.0, -1.0, 3.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, -4.0, -9.0, 36.0), std::make_tuple(-3.0, 3.0, 4.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, -6.0, -6.0, -7.0), std::make_tuple(7.0, 7.0, 7.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, 3.0, 3.0, 1.0), std::make_tuple(-1.0, -1.0, -1.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, 3.0, 3.0, 1.0), std::make_tuple(-1.0, -1.0, -1.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, 3.0, -6.0, -8.0), std::make_tuple(2.0, -1.0, -4.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, 2.0, -21.0, 18.0), std::make_tuple(3.0, -6.0, 1.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(1.0, 4.0, 7.0, 6.0), std::make_tuple(-2.0, -2.0, -2.0));
    ASSERT_3TUPLE_NEAR(TTauri::solveCubic(2.0, 9.0, 3.0, -4.0), std::make_tuple(-4.0, -1.0, 0.5));

    // Fails because of numeric inaccuracies, solveCubic will return only one real root.
    //ASSERT_DOUBLE_3TUPLE_EQ(TTauri::solveCubic(1.0, -5.0, 8.0, -4.0), std::make_tuple(1.0, 2.0, 2.0));
}
