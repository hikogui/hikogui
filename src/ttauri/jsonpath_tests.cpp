// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/jsonpath.hpp"
#include <gtest/gtest.h>

using namespace tt;

TEST(jsonpath, parse)
{
    ASSERT_EQ(to_string(parse_jsonpath("$.store.book[*].author")), "$['store']['book'][*]['author']"s);
    ASSERT_EQ(to_string(parse_jsonpath("$..author")), "$..['author']"s);
    ASSERT_EQ(to_string(parse_jsonpath("$.store.*")), "$['store'][*]"s);
    ASSERT_EQ(to_string(parse_jsonpath("$['store'].*")), "$['store'][*]"s);
    ASSERT_EQ(to_string(parse_jsonpath("$['store','author'].*")), "$['store','author'][*]"s);
    ASSERT_EQ(to_string(parse_jsonpath("$.store..price")), "$['store']..['price']"s);
    ASSERT_EQ(to_string(parse_jsonpath("$..book[2]")), "$..['book'][2]"s);
    ASSERT_EQ(to_string(parse_jsonpath("$..book[-1:]")), "$..['book'][-1:e:1]"s);
    ASSERT_EQ(to_string(parse_jsonpath("$..book[0,1]")), "$..['book'][0,1]"s);
    ASSERT_EQ(to_string(parse_jsonpath("$..book[:2]")), "$..['book'][0:2:1]"s);
    ASSERT_EQ(to_string(parse_jsonpath("$..*")), "$..[*]"s);
}
