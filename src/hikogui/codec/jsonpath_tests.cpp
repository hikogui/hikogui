// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "jsonpath.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>

using namespace hi;

TEST(jsonpath, parse)
{
    ASSERT_EQ(to_string(jsonpath("$.store.book[*].author")), "$['store']['book'][*]['author']");
    ASSERT_EQ(to_string(jsonpath("$..author")), "$..['author']");
    ASSERT_EQ(to_string(jsonpath("$.store.*")), "$['store'][*]");
    ASSERT_EQ(to_string(jsonpath("$['store'].*")), "$['store'][*]");
    ASSERT_EQ(to_string(jsonpath("$['store','author'].*")), "$['store','author'][*]");
    ASSERT_EQ(to_string(jsonpath("$.store..price")), "$['store']..['price']");
    ASSERT_EQ(to_string(jsonpath("$..book[2]")), "$..['book'][2]");
    ASSERT_EQ(to_string(jsonpath("$..book[-1]")), "$..['book'][-1]");
    ASSERT_EQ(to_string(jsonpath("$..book[-1:]")), "$..['book'][-1:e:1]");
    ASSERT_EQ(to_string(jsonpath("$..book[0,1]")), "$..['book'][0,1]");
    ASSERT_EQ(to_string(jsonpath("$..book[:2]")), "$..['book'][0:2:1]");
    ASSERT_EQ(to_string(jsonpath("$..*")), "$..[*]");
}
