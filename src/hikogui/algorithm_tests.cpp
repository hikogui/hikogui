// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/algorithm.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(algorithm, shuffle_by_index1)
{
    auto items = std::string{"abcde"};
    hilet indices = std::vector<int>{4, 3, 2, 1, 0};

    auto last = shuffle_by_index(begin(items), end(items), begin(indices), end(indices));
    ASSERT_EQ(last, end(items));
    ASSERT_EQ(items, "edcba");
}

TEST(algorithm, shuffle_by_index2)
{
    auto items = std::string{"abcde"};
    hilet indices = std::vector<int>{4, 3, 2};

    auto last = shuffle_by_index(begin(items), end(items), begin(indices), end(indices));
    items.erase(last, end(items));

    ASSERT_EQ(last, end(items));
    ASSERT_EQ(items, "edc");
}

TEST(algorithm, shuffle_by_index3)
{
    auto items = std::string{"abcde"};
    hilet indices = std::vector<int>{0, 1, 3, 2, 4};

    auto last = shuffle_by_index(begin(items), end(items), begin(indices), end(indices));
    ASSERT_EQ(last, end(items));
    ASSERT_EQ(items, "abdce");
}

TEST(algorithm, shuffle_by_index4)
{
    auto items = std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    hilet indices = std::vector<int>{12, 13, 11, 10, 9, 7, 8, 6, 5, 4, 3, 2, 1, 0};

    auto last = shuffle_by_index(begin(items), end(items), begin(indices), end(indices));
    ASSERT_EQ(last, end(items));
    ASSERT_EQ(items, indices);
}
