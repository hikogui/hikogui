// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/counters.hpp"
#include "ttauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(Counters, CompileTime) {
    increment_counter<"foo_a">();
    increment_counter<"bar_a">();
    increment_counter<"bar_a">();

    ASSERT_EQ(read_counter<"baz_a">(), 0);
    ASSERT_EQ(read_counter<"foo_a">(), 1);
    ASSERT_EQ(read_counter<"bar_a">(), 2);
}

TEST(Counters, RunTimeRead) {
    increment_counter<"foo_b">();
    increment_counter<"bar_b">();
    increment_counter<"bar_b">();

    ASSERT_EQ(read_counter("baz_b").first, 0);
    ASSERT_EQ(read_counter("foo_b").first, 1);
    ASSERT_EQ(read_counter("bar_b").first, 2);
}
