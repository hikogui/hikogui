// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/counters.hpp"
#include "hikogui/utility.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(Counters, CompileTime)
{
    ++global_counter<"foo_a">;
    ++global_counter<"bar_a">;
    ++global_counter<"bar_a">;

    ASSERT_EQ(global_counter<"baz_a">, 0);
    ASSERT_EQ(global_counter<"foo_a">, 1);
    ASSERT_EQ(global_counter<"bar_a">, 2);
}

TEST(Counters, RunTimeRead)
{
    ++global_counter<"foo_b">;
    ++global_counter<"bar_b">;
    ++global_counter<"bar_b">;

    ASSERT_EQ(get_global_counter_if("baz_b"), nullptr);
    ASSERT_EQ(*get_global_counter_if("foo_b"), 1);
    ASSERT_EQ(*get_global_counter_if("bar_b"), 2);
}
