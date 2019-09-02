// Copyright 2019 Pokitec
// All rights reserved.

#include "datum.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace TTauri;

TEST(Datum, IntOperations) {
    let v = datum{42};

    ASSERT_EQ(static_cast<int>(v), 42);
    ASSERT_EQ(static_cast<float>(v), 42.0);
    ASSERT_EQ(static_cast<std::string>(v), "42"s);
    ASSERT_EQ(static_cast<bool>(v), true);

    ASSERT_EQ(v.is_integer(), true);
    ASSERT_EQ(v.is_numeric(), true);
    ASSERT_EQ(v.is_float(), false);
    ASSERT_EQ(v.is_boolean(), false);
    ASSERT_EQ(v.is_string(), false);
    ASSERT_EQ(v.is_url(), false);

    ASSERT_EQ(v == 42, true);
    ASSERT_EQ(v < 42, false);
    ASSERT_EQ(v < 41, false);
    ASSERT_EQ(v < 43, true);

    ASSERT_EQ(v == 42.0, true);
    ASSERT_EQ(v < 42.0, false);
    ASSERT_EQ(v < 41.0, false);
    ASSERT_EQ(v < 43.0, true);

    let a = v + 3;
    ASSERT_EQ(a.is_integer(), true);
    ASSERT_EQ(a == 45, true);

    let b = v + 3.0;
    ASSERT_EQ(b.is_float(), true);
    ASSERT_EQ(b == 45.0, true);

}

TEST(Datum, FloatOperations) {
    let v = datum{42.0};

    ASSERT_EQ(static_cast<int>(v), 42);
    ASSERT_EQ(static_cast<float>(v), 42.0);
    ASSERT_EQ(static_cast<std::string>(v), "42.0"s);
    ASSERT_EQ(static_cast<bool>(v), true);

    ASSERT_EQ(v.is_integer(), false);
    ASSERT_EQ(v.is_numeric(), true);
    ASSERT_EQ(v.is_float(), true);
    ASSERT_EQ(v.is_boolean(), false);
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

    let a = v + 3;
    ASSERT_EQ(a.is_float(), true);
    ASSERT_EQ(a == 45.0, true);

    let b = v + 3.0;
    ASSERT_EQ(a.is_float(), true);
    ASSERT_EQ(a == 45.0, true);

}

TEST(Datum, StringOperations) {
    let v = datum{"Hello World"};

    ASSERT_EQ(static_cast<std::string>(v), "Hello World"s);
}
