// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "defer.hpp"
#include <gtest/gtest.h>

TEST(defer, early_out)
{
    int a = 0;
    int b = 0;

    do {
        hilet d_a = hi::defer([&]{ a = 42; });
        ASSERT_EQ(a, 0);

        // If will be taken.
        if (a == 0) {
            break;
        }

        hilet d_b = hi::defer([&]{ b = a + 1; });
    } while (false);

    ASSERT_EQ(a, 42);
    ASSERT_EQ(b, 0);
}

TEST(defer, fully)
{
    int a = 0;
    int b = 0;

    do {
        hilet d_a = hi::defer([&]{ a = 42; });
        ASSERT_EQ(a, 0);

        // If will NOT be taken.
        if (a == 42) {
            break;
        }

        hilet d_b = hi::defer([&]{ b = a + 5; });
        ASSERT_EQ(b, 0);
    } while (false);

    // d_b destructor will be called before, d_a, this is when a is still zero.
    ASSERT_EQ(a, 42);
    ASSERT_EQ(b, 5);
}

