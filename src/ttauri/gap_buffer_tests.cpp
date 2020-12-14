// Copyright 2019 Pokitec
// All rights reserved.

#include "gap_buffer.hpp"
#include "required.hpp"
#include "hash.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;

TEST(gap_buffer, default_constructor)
{
    auto tmp = gap_buffer<int>{};
    ASSERT_EQ(tmp, gap_buffer<int>{});
}

TEST(gap_buffer, list_initialization)
{
    auto tmp = gap_buffer<int>{1, 2, 3};
    ASSERT_EQ(tmp.size(), 3);
    ASSERT_EQ(tmp.size(), 3);
    ASSERT_EQ(tmp[0], 1);
    ASSERT_EQ(tmp[1], 2);
    ASSERT_EQ(tmp[2], 3);

    auto e = gap_buffer<int>{1, 2, 3};
    ASSERT_EQ(tmp, e);
}

TEST(gap_buffer, push_back)
{
    auto tmp = gap_buffer<int>{};
    auto e = std::vector<int>{};
    ASSERT_EQ(tmp, e);

    // Push back to at least two reallocation.
    for (auto i = 0; i != 500; ++i) {
        tmp.push_back(i * 3);
        e.push_back(i * 3);
        ASSERT_EQ(tmp, e);
    }
}

TEST(gap_buffer, push_front)
{
    auto tmp = gap_buffer<int>{};
    auto e = std::vector<int>{};
    ASSERT_EQ(tmp, e);

    // Push back to at least two reallocation.
    for (auto i = 0; i != 500; ++i) {
        tmp.push_front(i * 3);
        e.insert(e.begin(), i * 3);
        ASSERT_EQ(tmp, e);
    }
}

TEST(gap_buffer, insert_before)
{
    auto tmp = gap_buffer<int>{};
    auto e = std::vector<int>{};
    ASSERT_EQ(tmp, e);

    // Insert to at least two reallocation.
    for (size_t i = 0; i != 500; ++i) {
        // Get a semi random number indexing into the current gap_buffer and vector, or one beyond.
        auto index = hash_mix_two(i, i) % (i + 1);

        tmp.insert_before(tmp.begin() + index, narrow_cast<int>(i * 3));
        e.insert(e.begin() + index, narrow_cast<int>(i * 3));
        ASSERT_EQ(tmp, e);
    }
}

TEST(gap_buffer, insert_after)
{
    auto tmp = gap_buffer<int>{1};
    auto e = std::vector<int>{1};
    ASSERT_EQ(tmp, e);

    // Insert to at least two reallocation.
    for (size_t i = 1; i != 500; ++i) {
        // Get a semi random number indexing into the current gap_buffer and vector, or one beyond.
        auto index = hash_mix_two(i, i) % i;

        tmp.insert_after(tmp.begin() + index, narrow_cast<int>(i * 3));
        e.insert(e.begin() + index + 1, narrow_cast<int>(i * 3));
        ASSERT_EQ(tmp, e);
    }
}
