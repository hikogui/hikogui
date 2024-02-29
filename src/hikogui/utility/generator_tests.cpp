// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "generator.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(generator_suite) {

hi::generator<int> my_generator()
{
    co_yield 42;
    co_yield 3;
    co_yield 12;
}

TEST_CASE(generator_test)
{
    auto test = my_generator();

    auto index = 0;
    for (auto number : test) {
        if (index == 0) {
            REQUIRE(number == 42);
        } else if (index == 1) {
            REQUIRE(number == 3);
        } else if (index == 2) {
            REQUIRE(number == 12);
        } else {
            REQUIRE(false);
        }
        ++index;
    }
}

TEST_CASE(generator_temporary_test)
{
    auto index = 0;
    for (auto number : my_generator()) {
        if (index == 0) {
            REQUIRE(number == 42);
        } else if (index == 1) {
            REQUIRE(number == 3);
        } else if (index == 2) {
            REQUIRE(number == 12);
        } else {
            REQUIRE(false);
        }
        ++index;
    }
}

};
