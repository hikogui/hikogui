// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/algorithm.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(algorithm, shuffle_by_index1)
{
    auto items = std::string{"abcde"};
    ttlet indices = std::vector<int>{4, 3, 2, 1, 0};

    auto last = shuffle_by_index(std::begin(items), std::end(items), std::begin(indices), std::end(indices));
    ASSERT_EQ(last, std::end(items));
    ASSERT_EQ(items, "edcba");
}

TEST(algorithm, shuffle_by_index2)
{
    auto items = std::string{"abcde"};
    ttlet indices = std::vector<int>{4, 3, 2};

    auto last = shuffle_by_index(std::begin(items), std::end(items), std::begin(indices), std::end(indices));
    items.erase(last, std::end(items));

    ASSERT_EQ(last, std::end(items));
    ASSERT_EQ(items, "edc");
}

TEST(algorithm, shuffle_by_index3)
{
    auto items = std::string{"abcde"};
    ttlet indices = std::vector<int>{0, 1, 3, 2, 4};

    auto last = shuffle_by_index(std::begin(items), std::end(items), std::begin(indices), std::end(indices));
    ASSERT_EQ(last, std::end(items));
    ASSERT_EQ(items, "abdce");
}
