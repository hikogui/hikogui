// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "generator.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

generator<int> my_generator()
{
    co_yield 42;
    co_yield 3;
    co_yield 12;
}

TEST(concepts, generator)
{
    auto test = my_generator();

    auto index = 0;
    for (auto number : test) {
        if (index == 0) {
            ASSERT_EQ(number, 42);
        } else if (index == 1) {
            ASSERT_EQ(number, 3);
        } else if (index == 2) {
            ASSERT_EQ(number, 12);
        } else {
            FAIL();
        }
        ++index;
    }
}

TEST(concepts, generator_temporary)
{
    auto index = 0;
    for (auto number : my_generator()) {
        if (index == 0) {
            ASSERT_EQ(number, 42);
        } else if (index == 1) {
            ASSERT_EQ(number, 3);
        } else if (index == 2) {
            ASSERT_EQ(number, 12);
        } else {
            FAIL();
        }
        ++index;
    }
}
