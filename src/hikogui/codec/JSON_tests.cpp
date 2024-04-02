// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "JSON.hpp"
#include "../utility/utility.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(JSON_suite) {

TEST_CASE(ParseEmpty)
{
    REQUIRE(hi::parse_JSON("{}") == hi::datum::make_map());
}

TEST_CASE(ParseInteger)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = 42;
    REQUIRE(hi::parse_JSON("{\"foo\": 42}") == expected);
}

TEST_CASE(ParseFloat)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = 42.0;
    REQUIRE(hi::parse_JSON("{\"foo\": 42.0}") == expected);
}

TEST_CASE(ParseString)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = "bar";
    REQUIRE(hi::parse_JSON("{\"foo\": \"bar\"}") == expected);
}

TEST_CASE(ParseBooleanTrue)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = true;
    REQUIRE(hi::parse_JSON("{\"foo\": true}") == expected);
}

TEST_CASE(ParseBooleanFalse)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = false;
    REQUIRE(hi::parse_JSON("{\"foo\": false}") == expected);
}

TEST_CASE(ParseNull)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = hi::datum{nullptr};
    REQUIRE(hi::parse_JSON("{\"foo\": null}") == expected);
}

TEST_CASE(ParseEmptyArray)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = hi::datum::make_vector();
    REQUIRE(hi::parse_JSON("{\"foo\": []}") == expected);
}

TEST_CASE(ParseSingleItemArray)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = hi::datum::make_vector(42);
    REQUIRE(hi::parse_JSON("{\"foo\": [42]}") == expected);
    REQUIRE(hi::parse_JSON("{\"foo\": [42,]}") == expected);
}

TEST_CASE(ParseTwoItemArray)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = hi::datum::make_vector(42, 43);
    REQUIRE(hi::parse_JSON("{\"foo\": [42, 43]}") == expected);
    REQUIRE(hi::parse_JSON("{\"foo\": [42, 43,]}") == expected);
}

TEST_CASE(ParseEmptyObject)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = hi::datum::make_map();
    REQUIRE(hi::parse_JSON("{\"foo\": {}}") == expected);
}

TEST_CASE(ParseSingleItemObject)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = hi::datum::make_map();
    expected["foo"]["bar"] = 42;
    REQUIRE(hi::parse_JSON("{\"foo\": {\"bar\": 42}}") == expected);
    REQUIRE(hi::parse_JSON("{\"foo\": {\"bar\": 42,}}") == expected);
}

TEST_CASE(ParseTwoItemObject)
{
    auto expected = hi::datum::make_map();
    expected["foo"] = hi::datum::make_map();
    expected["foo"]["bar"] = 42;
    expected["foo"]["baz"] = 43;
    REQUIRE(hi::parse_JSON("{\"foo\": {\"bar\": 42, \"baz\": 43}}") == expected);
    REQUIRE(hi::parse_JSON("{\"foo\": {\"bar\": 42, \"baz\": 43,}}") == expected);
}

};
