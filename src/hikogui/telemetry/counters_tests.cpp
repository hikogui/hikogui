// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "counters.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(counters) {

TEST_CASE(template_read)
{
    hi::global_counter<"foo_a"> = 0;
    hi::global_counter<"bar_a"> = 0;

    ++hi::global_counter<"foo_a">;
    ++hi::global_counter<"bar_a">;
    ++hi::global_counter<"bar_a">;

    REQUIRE(hi::global_counter<"baz_a"> == 0);
    REQUIRE(hi::global_counter<"foo_a"> == 1);
    REQUIRE(hi::global_counter<"bar_a"> == 2);
}

TEST_CASE(search_and_read)
{
    hi::global_counter<"foo_b"> = 0;
    hi::global_counter<"bar_b"> = 0;

    ++hi::global_counter<"foo_b">;
    ++hi::global_counter<"bar_b">;
    ++hi::global_counter<"bar_b">;

    REQUIRE(hi::get_global_counter_if("baz_b") == nullptr);
    REQUIRE(*hi::get_global_counter_if("foo_b") == 1);
    REQUIRE(*hi::get_global_counter_if("bar_b") == 2);
}

};