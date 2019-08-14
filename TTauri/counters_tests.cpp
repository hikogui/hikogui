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
    increment_counter<"foo-a"_tag>();
    increment_counter<"bar-a"_tag>();
    increment_counter<"bar-a"_tag>();

    ASSERT_EQ(read_counter<"baz-a"_tag>(), 0);
    ASSERT_EQ(read_counter<"foo-a"_tag>(), 1);
    ASSERT_EQ(read_counter<"bar-a"_tag>(), 2);
}

TEST(Counters, RunTimeRead) {
    increment_counter<"foo-b"_tag>();
    increment_counter<"bar-b"_tag>();
    increment_counter<"bar-b"_tag>();

    ASSERT_EQ(read_counter("baz-b"_tag), 0);
    ASSERT_EQ(read_counter("foo-b"_tag), 1);
    ASSERT_EQ(read_counter("bar-b"_tag), 2);
}

TEST(Counters, RunTimeReadWithStrings) {
    increment_counter<"foo-c"_tag>();
    increment_counter<"bar-c"_tag>();
    increment_counter<"bar-c"_tag>();

    ASSERT_EQ(read_counter("baz-c"), 0);
    ASSERT_EQ(read_counter("foo-c"), 1);
    ASSERT_EQ(read_counter("bar-c"), 2);
}