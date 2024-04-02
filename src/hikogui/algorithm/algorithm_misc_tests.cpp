// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "algorithm_misc.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(algorithm_suite)
{

TEST_CASE(shuffle_by_index1)
{
    auto items = std::string{"abcde"};
    auto const indices = std::vector<int>{4, 3, 2, 1, 0};

    auto last = hi::shuffle_by_index(begin(items), end(items), begin(indices), end(indices));
    REQUIRE(last == end(items));
    REQUIRE(items == "edcba");
}

TEST_CASE(shuffle_by_index2)
{
    auto items = std::string{"abcde"};
    auto const indices = std::vector<int>{4, 3, 2};

    auto last = hi::shuffle_by_index(begin(items), end(items), begin(indices), end(indices));
    items.erase(last, end(items));

    REQUIRE(last == end(items));
    REQUIRE(items == "edc");
}

TEST_CASE(shuffle_by_index3)
{
    auto items = std::string{"abcde"};
    auto const indices = std::vector<int>{0, 1, 3, 2, 4};

    auto last = hi::shuffle_by_index(begin(items), end(items), begin(indices), end(indices));
    REQUIRE(last == end(items));
    REQUIRE(items == "abdce");
}

TEST_CASE(shuffle_by_index4)
{
    auto items = std::vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    auto const indices = std::vector<int>{12, 13, 11, 10, 9, 7, 8, 6, 5, 4, 3, 2, 1, 0};

    auto last = hi::shuffle_by_index(begin(items), end(items), begin(indices), end(indices));
    REQUIRE(last == end(items));
    REQUIRE(items == indices);
}

};
