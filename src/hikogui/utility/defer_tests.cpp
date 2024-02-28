// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "defer.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(defer) {

TEST_CASE(early_out)
{
    int a = 0;
    int b = 0;

    do {
        auto const d_a = hi::defer([&]{ a = 42; });
        REQUIRE(a == 0);

        // If will be taken.
        if (a == 0) {
            break;
        }

        auto const d_b = hi::defer([&]{ b = a + 1; });
    } while (false);

    REQUIRE(a == 42);
    REQUIRE(b == 0);
}

TEST_CASE(fully)
{
    int a = 0;
    int b = 0;

    do {
        auto const d_a = hi::defer([&]{ a = 42; });
        REQUIRE(a == 0);

        // If will NOT be taken.
        if (a == 42) {
            break;
        }

        auto const d_b = hi::defer([&]{ b = a + 5; });
        REQUIRE(b == 0);
    } while (false);

    // d_b destructor will be called before, d_a, this is when a is still zero.
    REQUIRE(a == 42);
    REQUIRE(b == 5);
}

};
