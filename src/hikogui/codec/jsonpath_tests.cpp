// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "jsonpath.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(jsonpath_suite) {

TEST_CASE(parse)
{
    REQUIRE(to_string(hi::jsonpath("$.store.book[*].author")) == "$['store']['book'][*]['author']");
    REQUIRE(to_string(hi::jsonpath("$..author")) == "$..['author']");
    REQUIRE(to_string(hi::jsonpath("$.store.*")) == "$['store'][*]");
    REQUIRE(to_string(hi::jsonpath("$['store'].*")) == "$['store'][*]");
    REQUIRE(to_string(hi::jsonpath("$['store','author'].*")) == "$['store','author'][*]");
    REQUIRE(to_string(hi::jsonpath("$.store..price")) == "$['store']..['price']");
    REQUIRE(to_string(hi::jsonpath("$..book[2]")) == "$..['book'][2]");
    REQUIRE(to_string(hi::jsonpath("$..book[-1]")) == "$..['book'][-1]");
    REQUIRE(to_string(hi::jsonpath("$..book[-1:]")) == "$..['book'][-1:e:1]");
    REQUIRE(to_string(hi::jsonpath("$..book[0,1]")) == "$..['book'][0,1]");
    REQUIRE(to_string(hi::jsonpath("$..book[:2]")) == "$..['book'][0:2:1]");
    REQUIRE(to_string(hi::jsonpath("$..*")) == "$..[*]");
}

};
