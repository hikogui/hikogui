// Copyright 2019 Pokitec
// All rights reserved.

#include "counters.hpp"
#include "required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(Counters, CompileTime) {
    increment_counter<"foo_a"_tag>();
    increment_counter<"bar_a"_tag>();
    increment_counter<"bar_a"_tag>();

    ASSERT_EQ(read_counter<"baz_a"_tag>(), 0);
    ASSERT_EQ(read_counter<"foo_a"_tag>(), 1);
    ASSERT_EQ(read_counter<"bar_a"_tag>(), 2);
}

TEST(Counters, RunTimeRead) {
    increment_counter<"foo_b"_tag>();
    increment_counter<"bar_b"_tag>();
    increment_counter<"bar_b"_tag>();

    ASSERT_EQ(read_counter("baz_b"_tag), 0);
    ASSERT_EQ(read_counter("foo_b"_tag), 1);
    ASSERT_EQ(read_counter("bar_b"_tag), 2);
}

TEST(Counters, RunTimeReadWithStrings) {
    increment_counter<"foo_c"_tag>();
    increment_counter<"bar_c"_tag>();
    increment_counter<"bar_c"_tag>();

    ASSERT_EQ(read_counter("baz_c"), 0);
    ASSERT_EQ(read_counter("foo_c"), 1);
    ASSERT_EQ(read_counter("bar_c"), 2);
}