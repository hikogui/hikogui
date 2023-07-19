// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "strings.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(String, Split)
{
    hilet result = split("path1/path2", '/');
    hilet check_value = std::vector<std::string>{"path1", "path2"};
    ASSERT_EQ(result, check_value);
}

TEST(String, Normalize_LF)
{
    ASSERT_EQ(normalize_lf("hello\nworld\n\nFoo\n"), "hello\nworld\n\nFoo\n");
    ASSERT_EQ(normalize_lf("hello\rworld\r\rFoo\r"), "hello\nworld\n\nFoo\n");
    ASSERT_EQ(normalize_lf("hello\r\nworld\r\n\r\nFoo\r\n"), "hello\nworld\n\nFoo\n");
}
