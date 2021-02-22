// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/datum.hpp"
#include "ttauri/exception.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace tt;

TEST(Datum, IntOperations) {
    ttlet v = datum{42};

    ASSERT_EQ(static_cast<int>(v), 42);
    ASSERT_EQ(static_cast<float>(v), 42.0);
    ASSERT_EQ(static_cast<std::string>(v), "42"s);
    ASSERT_EQ(static_cast<bool>(v), true);

    ASSERT_EQ(v.is_integer(), true);
    ASSERT_EQ(v.is_numeric(), true);
    ASSERT_EQ(v.is_float(), false);
    ASSERT_EQ(v.is_bool(), false);
    ASSERT_EQ(v.is_string(), false);
    ASSERT_EQ(v.is_url(), false);

    ASSERT_EQ(v == 42, true);
    ASSERT_EQ(v < 42, false);
    ASSERT_EQ(v < 41, false);
    ASSERT_EQ(v < 43, true);
    ASSERT_EQ(v - 5, 37);

    ASSERT_EQ(v == 42.0, true);
    ASSERT_EQ(v < 42.0, false);
    ASSERT_EQ(v < 41.0, false);
    ASSERT_EQ(v < 43.0, true);


    ttlet a = v + 3;
    ASSERT_EQ(a.is_integer(), true);
    ASSERT_EQ(a == 45, true);

    ttlet b = v + 3.0;
    ASSERT_EQ(b.is_float(), true);
    ASSERT_EQ(b == 45.0, true);

    ASSERT_THROW(datum(-42) << 1, operation_error);
    ASSERT_THROW(datum(-42) >> -1, operation_error);

    ASSERT_EQ(datum(42) << 0, 42);
    ASSERT_EQ(datum(42) >> 0, 42);
    ASSERT_EQ(datum(42) << 1, 84);
    ASSERT_EQ(datum(42) >> -1, 84);
    ASSERT_EQ(datum(-42) >> 1, -21);

    ASSERT_EQ(datum(42) << 63, 0);
    ASSERT_EQ(datum(42) >> 63, 0);
    ASSERT_EQ(datum(-42) >> 63, -1);
    ASSERT_EQ(datum(42) << 64, 0);
    ASSERT_EQ(datum(42) >> 64, 0);
    ASSERT_EQ(datum(-42) >> 64, -1);
}

TEST(Datum, DecimalOperations) {

    ttlet v = decimal(-25);
    ASSERT_EQ(static_cast<decimal>(datum{v}), v);
}


TEST(Datum, NegativeIntOperations) {
    ttlet v = datum{-1};

    ASSERT_EQ(static_cast<int>(v), -1);
    ASSERT_EQ(static_cast<std::string>(v), "-1"s);
}

TEST(Datum, FloatOperations) {
    ttlet v = datum{42.0};

    ASSERT_EQ(static_cast<int>(v), 42);
    ASSERT_EQ(static_cast<float>(v), 42.0);
    ASSERT_EQ(static_cast<std::string>(v), "42.0"s);
    ASSERT_EQ(static_cast<bool>(v), true);

    ASSERT_EQ(v.is_integer(), false);
    ASSERT_EQ(v.is_numeric(), true);
    ASSERT_EQ(v.is_float(), true);
    ASSERT_EQ(v.is_bool(), false);
    ASSERT_EQ(v.is_string(), false);
    ASSERT_EQ(v.is_url(), false);

    ASSERT_EQ(v == 42.0, true);
    ASSERT_EQ(v < 42.0, false);
    ASSERT_EQ(v < 41.0, false);
    ASSERT_EQ(v < 43.0, true);

    ASSERT_EQ(v == 42, true);
    ASSERT_EQ(v < 42, false);
    ASSERT_EQ(v < 41, false);
    ASSERT_EQ(v < 43, true);

    ttlet a = v + 3;
    ASSERT_EQ(a.is_float(), true);
    ASSERT_EQ(a == 45.0, true);

    ttlet b = v + 3.0;
    ASSERT_EQ(a.is_float(), true);
    ASSERT_EQ(a == 45.0, true);

}

TEST(Datum, StringOperations) {
    ttlet v = datum{"Hello World"};

    ASSERT_EQ(static_cast<std::string>(v), "Hello World"s);
}

TEST(Datum, WillCastTo) {
    ttlet v = datum{-1};

    ASSERT_TRUE(will_cast_to<int>(v));
    ASSERT_FALSE(will_cast_to<URL>(v));
}

TEST(Datum, ArrayOperations) {
    ttlet v = datum{datum::vector{11, 12, 13, 14, 15}};

    ASSERT_EQ(v[0], 11);
    ASSERT_EQ(v[1], 12);
    ASSERT_EQ(v[2], 13);
    ASSERT_EQ(v[3], 14);
    ASSERT_EQ(v[4], 15);
    ASSERT_THROW(v[5], operation_error);

    ASSERT_THROW(v[-6], operation_error);
    ASSERT_EQ(v[-5], 11);
    ASSERT_EQ(v[-4], 12);
    ASSERT_EQ(v[-3], 13);
    ASSERT_EQ(v[-2], 14);
    ASSERT_EQ(v[-1], 15);
}
