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

    ASSERT_EQ(holds_alternative<long long>(v), true);
    ASSERT_EQ(holds_alternative<double>(v), false);
    ASSERT_EQ(holds_alternative<decimal>(v), false);
    ASSERT_EQ(holds_alternative<URL>(v), false);
    ASSERT_EQ(holds_alternative<std::string>(v), false);

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
    ASSERT_EQ(holds_alternative<long long>(a), true);
    ASSERT_EQ(a == 45, true);

    ttlet b = v + 3.0;
    ASSERT_EQ(holds_alternative<double>(b), true);
    ASSERT_EQ(b == 45.0, true);

    ASSERT_THROW((void)(datum(-42) >> -1), std::domain_error);
    ASSERT_THROW((void)(datum(42) >> -1), std::domain_error);

    ASSERT_EQ(datum(42) << 0, 42);
    ASSERT_EQ(datum(42) >> 0, 42);
    ASSERT_EQ(datum(42) << 1, 84);
    ASSERT_EQ(datum(-42) >> 1, -21);
    ASSERT_EQ(datum(-42) << 1, -84);

    ASSERT_EQ(datum(42) << 63, 0);
    ASSERT_EQ(datum(42) >> 63, 0);
    ASSERT_EQ(datum(-42) >> 63, -1);
    ASSERT_THROW((void)(datum(42) << 64), std::domain_error);
    ASSERT_THROW((void)(datum(42) >> 64), std::domain_error);
    ASSERT_THROW((void)(datum(-42) >> 64), std::domain_error);
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
    ASSERT_EQ(static_cast<std::string>(v), "42"s);
    ASSERT_EQ(to_string(v), "42"s);
    ASSERT_EQ(std::format("{}", v), "42"s);
    ASSERT_EQ(repr(v), "42.0"s);
    ASSERT_EQ(static_cast<bool>(v), true);

    ASSERT_EQ(v == 42.0, true);
    ASSERT_EQ(v < 42.0, false);
    ASSERT_EQ(v < 41.0, false);
    ASSERT_EQ(v < 43.0, true);

    ASSERT_EQ(v == 42, true);
    ASSERT_EQ(v < 42, false);
    ASSERT_EQ(v < 41, false);
    ASSERT_EQ(v < 43, true);

    ttlet a = v + 3;
    ASSERT_EQ(holds_alternative<double>(a), true);
    ASSERT_EQ(a == 45.0, true);

    ttlet b = v + 3.0;
    ASSERT_EQ(holds_alternative<double>(b), true);
    ASSERT_EQ(b == 45.0, true);

}

TEST(Datum, StringOperations) {
    ttlet v = datum{"Hello World"};

    ASSERT_EQ(static_cast<std::string>(v), "Hello World"s);
}

TEST(Datum, ArrayOperations) {
    ttlet v = datum::make_vector(11, 12, 13, 14, 15);

    ASSERT_EQ(v[0], 11);
    ASSERT_EQ(v[1], 12);
    ASSERT_EQ(v[2], 13);
    ASSERT_EQ(v[3], 14);
    ASSERT_EQ(v[4], 15);
    ASSERT_THROW((void)(v[5]), std::overflow_error);
    
    ASSERT_THROW((void)(v[-6]), std::overflow_error);
    ASSERT_EQ(v[-5], 11);
    ASSERT_EQ(v[-4], 12);
    ASSERT_EQ(v[-3], 13);
    ASSERT_EQ(v[-2], 14);
    ASSERT_EQ(v[-1], 15);
}
