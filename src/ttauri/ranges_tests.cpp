// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/ranges.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(ranges, split1)
{
    auto test = std::vector{1, 2, -1, 3, 4};
    auto sep = std::vector{-1};

    auto index = 0;
    for (auto result : tt::views::split(test, sep)) {
        if (index == 0) {
            auto expected = std::vector{1,2};
            ASSERT_TRUE(std::ranges::equal(result, expected));
        } else if (index == 1) {
            auto expected = std::vector{3, 4};
            ASSERT_TRUE(std::ranges::equal(result, expected));
        } else {
            FAIL();
        }
        ++index;
    }
}

TEST(ranges, split2)
{
    auto test = std::string{"Hello..World"};

    auto index = 0;
    for (auto word : tt::views::split(test, "..")) {
        if (index == 0) {
            ASSERT_EQ(word, std::string{"Hello"});
        } else if (index == 1) {
            ASSERT_EQ(word, std::string{"World"});
        } else {
            FAIL();
        }
        ++index;
    }
}

TEST(ranges, split3)
{
    auto test = std::string{"Hello"};

    auto index = 0;
    for (auto word : tt::views::split(test, "..")) {
        if (index == 0) {
            ASSERT_EQ(word, std::string{"Hello"});
        } else {
            FAIL();
        }
        ++index;
    }
}

TEST(ranges, split4)
{
    auto test = std::string{""};

    auto r = tt::views::split(test, "..");
    ASSERT_TRUE(std::begin(r) == std::end(r));
}
