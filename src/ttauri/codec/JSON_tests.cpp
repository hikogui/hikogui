// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/codec/JSON.hpp"
#include "ttauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;


TEST(JSON, ParseEmpty) {
    ASSERT_EQ(parseJSON("{}"), datum::map{});
}

TEST(JSON, ParseInteger) {
    auto expected = datum::map{};
    expected["foo"] = 42;
    ASSERT_EQ(parseJSON("{\"foo\": 42}"), expected);
}

TEST(JSON, ParseFloat) {
    auto expected = datum::map{};
    expected["foo"] = 42.0;
    ASSERT_EQ(parseJSON("{\"foo\": 42.0}"), expected);
}

TEST(JSON, ParseString) {
    auto expected = datum::map{};
    expected["foo"] = "bar";
    ASSERT_EQ(parseJSON("{\"foo\": \"bar\"}"), expected);
}

TEST(JSON, ParseBooleanTrue) {
    auto expected = datum::map{};
    expected["foo"] = true;
    ASSERT_EQ(parseJSON("{\"foo\": true}"), expected);
}

TEST(JSON, ParseBooleanFalse) {
    auto expected = datum::map{};
    expected["foo"] = false;
    ASSERT_EQ(parseJSON("{\"foo\": false}"), expected);
}

TEST(JSON, ParseNull) {
    auto expected = datum::map{};
    expected["foo"] = datum::null{};
    ASSERT_EQ(parseJSON("{\"foo\": null}"), expected);
}

TEST(JSON, ParseEmptyArray) {
    auto expected = datum::map{};
    expected["foo"] = datum::vector{};
    ASSERT_EQ(parseJSON("{\"foo\": []}"), expected);
}

TEST(JSON, ParseSingleItemArray) {
    auto expected = datum::map{};
    expected["foo"] = datum::vector{42};
    ASSERT_EQ(parseJSON("{\"foo\": [42]}"), expected);
    ASSERT_EQ(parseJSON("{\"foo\": [42,]}"), expected);
}

TEST(JSON, ParseTwoItemArray) {
    auto expected = datum::map{};
    expected["foo"] = datum::vector{42, 43};
    ASSERT_EQ(parseJSON("{\"foo\": [42, 43]}"), expected);
    ASSERT_EQ(parseJSON("{\"foo\": [42, 43,]}"), expected);
}

TEST(JSON, ParseEmptyObject) {
    auto expected = datum::map{};
    expected["foo"] = datum::map{};
    ASSERT_EQ(parseJSON("{\"foo\": {}}"), expected);
}

TEST(JSON, ParseSingleItemObject) {
    auto expected = datum::map{};
    expected["foo"] = datum::map{};
    expected["foo"]["bar"] = 42;
    ASSERT_EQ(parseJSON("{\"foo\": {\"bar\": 42}}"), expected);
    ASSERT_EQ(parseJSON("{\"foo\": {\"bar\": 42,}}"), expected);
}

TEST(JSON, ParseTwoItemObject) {
    auto expected = datum::map{};
    expected["foo"] = datum::map{};
    expected["foo"]["bar"] = 42;
    expected["foo"]["baz"] = 43;
    ASSERT_EQ(parseJSON("{\"foo\": {\"bar\": 42, \"baz\": 43}}"), expected);
    ASSERT_EQ(parseJSON("{\"foo\": {\"bar\": 42, \"baz\": 43,}}"), expected);
}