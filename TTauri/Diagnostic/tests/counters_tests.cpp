// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Diagnostic/counters.hpp"
#include "TTauri/Required/required.hpp"
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

    ASSERT_EQ(read_counter("baz_b"_tag).first, 0);
    ASSERT_EQ(read_counter("foo_b"_tag).first, 1);
    ASSERT_EQ(read_counter("bar_b"_tag).first, 2);
}

TEST(Counters, RunTimeReadWithStrings) {
    increment_counter<"foo_c"_tag>();
    increment_counter<"bar_c"_tag>();
    increment_counter<"bar_c"_tag>();

    ASSERT_EQ(read_counter("baz_c").first, 0);
    ASSERT_EQ(read_counter("foo_c").first, 1);
    ASSERT_EQ(read_counter("bar_c").first, 2);
}
