// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "lookahead_iterator.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <iterator>

TEST(lookahead_iterator, iterate)
{
    auto values = std::vector{'a', 'b', 'c', 'd', 'e', 'f'};

    auto it = hi::make_lookahead_iterator<4>(values.begin(), values.end());

    ASSERT_EQ(it.size(), 4);
    ASSERT_FALSE(it.empty());
    ASSERT_NE(it, std::default_sentinel);
    ASSERT_EQ(*it, 'a');
    ASSERT_EQ(it[0], 'a');
    ASSERT_EQ(it[1], 'b');
    ASSERT_EQ(it[2], 'c');
    ASSERT_EQ(it[3], 'd');

    ++it;
    ASSERT_EQ(it.size(), 4);
    ASSERT_FALSE(it.empty());
    ASSERT_NE(it, std::default_sentinel);
    ASSERT_EQ(*it, 'b');
    ASSERT_EQ(it[0], 'b');
    ASSERT_EQ(it[1], 'c');
    ASSERT_EQ(it[2], 'd');
    ASSERT_EQ(it[3], 'e');

    ++it;
    ASSERT_EQ(it.size(), 4);
    ASSERT_FALSE(it.empty());
    ASSERT_NE(it, std::default_sentinel);
    ASSERT_EQ(*it, 'c');
    ASSERT_EQ(it[0], 'c');
    ASSERT_EQ(it[1], 'd');
    ASSERT_EQ(it[2], 'e');
    ASSERT_EQ(it[3], 'f');

    ++it;
    ASSERT_EQ(it.size(), 3);
    ASSERT_FALSE(it.empty());
    ASSERT_NE(it, std::default_sentinel);
    ASSERT_EQ(*it, 'd');
    ASSERT_EQ(it[0], 'd');
    ASSERT_EQ(it[1], 'e');
    ASSERT_EQ(it[2], 'f');

    ++it;
    ASSERT_EQ(it.size(), 2);
    ASSERT_FALSE(it.empty());
    ASSERT_NE(it, std::default_sentinel);
    ASSERT_EQ(*it, 'e');
    ASSERT_EQ(it[0], 'e');
    ASSERT_EQ(it[1], 'f');

    ++it;
    ASSERT_EQ(it.size(), 1);
    ASSERT_FALSE(it.empty());
    ASSERT_NE(it, std::default_sentinel);
    ASSERT_EQ(*it, 'f');
    ASSERT_EQ(it[0], 'f');

        ++it;
    ASSERT_EQ(it.size(), 0);
    ASSERT_TRUE(it.empty());
    ASSERT_EQ(it, std::default_sentinel);
}
