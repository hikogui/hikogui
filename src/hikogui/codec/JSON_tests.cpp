// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "JSON.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>



using namespace std;
using namespace hi;

TEST(JSON, ParseEmpty)
{
    ASSERT_EQ(parse_JSON("{}"), datum::make_map());
}

TEST(JSON, ParseInteger)
{
    auto expected = datum::make_map();
    expected["foo"] = 42;
    ASSERT_EQ(parse_JSON("{\"foo\": 42}"), expected);
}

TEST(JSON, ParseFloat)
{
    auto expected = datum::make_map();
    expected["foo"] = 42.0;
    ASSERT_EQ(parse_JSON("{\"foo\": 42.0}"), expected);
}

TEST(JSON, ParseString)
{
    auto expected = datum::make_map();
    expected["foo"] = "bar";
    ASSERT_EQ(parse_JSON("{\"foo\": \"bar\"}"), expected);
}

TEST(JSON, ParseBooleanTrue)
{
    auto expected = datum::make_map();
    expected["foo"] = true;
    ASSERT_EQ(parse_JSON("{\"foo\": true}"), expected);
}

TEST(JSON, ParseBooleanFalse)
{
    auto expected = datum::make_map();
    expected["foo"] = false;
    ASSERT_EQ(parse_JSON("{\"foo\": false}"), expected);
}

TEST(JSON, ParseNull)
{
    auto expected = datum::make_map();
    expected["foo"] = datum{nullptr};
    ASSERT_EQ(parse_JSON("{\"foo\": null}"), expected);
}

TEST(JSON, ParseEmptyArray)
{
    auto expected = datum::make_map();
    expected["foo"] = datum::make_vector();
    ASSERT_EQ(parse_JSON("{\"foo\": []}"), expected);
}

TEST(JSON, ParseSingleItemArray)
{
    auto expected = datum::make_map();
    expected["foo"] = datum::make_vector(42);
    ASSERT_EQ(parse_JSON("{\"foo\": [42]}"), expected);
    ASSERT_EQ(parse_JSON("{\"foo\": [42,]}"), expected);
}

TEST(JSON, ParseTwoItemArray)
{
    auto expected = datum::make_map();
    expected["foo"] = datum::make_vector(42, 43);
    ASSERT_EQ(parse_JSON("{\"foo\": [42, 43]}"), expected);
    ASSERT_EQ(parse_JSON("{\"foo\": [42, 43,]}"), expected);
}

TEST(JSON, ParseEmptyObject)
{
    auto expected = datum::make_map();
    expected["foo"] = datum::make_map();
    ASSERT_EQ(parse_JSON("{\"foo\": {}}"), expected);
}

TEST(JSON, ParseSingleItemObject)
{
    auto expected = datum::make_map();
    expected["foo"] = datum::make_map();
    expected["foo"]["bar"] = 42;
    ASSERT_EQ(parse_JSON("{\"foo\": {\"bar\": 42}}"), expected);
    ASSERT_EQ(parse_JSON("{\"foo\": {\"bar\": 42,}}"), expected);
}

TEST(JSON, ParseTwoItemObject)
{
    auto expected = datum::make_map();
    expected["foo"] = datum::make_map();
    expected["foo"]["bar"] = 42;
    expected["foo"]["baz"] = 43;
    ASSERT_EQ(parse_JSON("{\"foo\": {\"bar\": 42, \"baz\": 43}}"), expected);
    ASSERT_EQ(parse_JSON("{\"foo\": {\"bar\": 42, \"baz\": 43,}}"), expected);
}
