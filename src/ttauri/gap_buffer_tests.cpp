// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

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

std::pair<gap_buffer<int>, std::vector<int>> gap_buffer_test_initial_data(size_t nr_elements)
{
    std::pair<gap_buffer<int>, std::vector<int>> r;

    for (size_t i = 0; i != nr_elements; ++i) {
        r.first.push_back(narrow_cast<int>(i * 3));
        r.second.push_back(narrow_cast<int>(i * 3));
    }
    return r;
}

TEST(gap_buffer, pop_back)
{
    auto [tmp, e] = gap_buffer_test_initial_data(500);
    ASSERT_EQ(tmp, e);

    while (!tmp.empty()) {
        tmp.pop_back();
        e.pop_back();
        ASSERT_EQ(tmp, e);
    }
}

TEST(gap_buffer, pop_front)
{
    auto [tmp, e] = gap_buffer_test_initial_data(500);
    ASSERT_EQ(tmp, e);

    while (!tmp.empty()) {
        tmp.pop_front();
        e.erase(e.begin());
        ASSERT_EQ(tmp, e);
    }
}

TEST(gap_buffer, erase)
{
    auto [tmp, e] = gap_buffer_test_initial_data(500);
    ASSERT_EQ(tmp, e);

    while (!tmp.empty()) {
        // Get a semi random number indexing into the current gap_buffer and vector.
        ttlet size = tmp.size();
        ttlet index = hash_mix_two(size, size) % size;

        ttlet tmp_i = tmp.erase(tmp.begin() + index);
        ttlet e_i = e.erase(e.begin() + index);

        ASSERT_EQ(std::distance(tmp.begin(), tmp_i), std::distance(e.begin(), e_i));
        ASSERT_EQ(tmp, e);
    }
}

TEST(gap_buffer, insert_after_clear)
{
    ttlet start_size = 500;
    auto [tmp, e] = gap_buffer_test_initial_data(start_size);
    ASSERT_EQ(tmp, e);

    tmp.clear();
    e.clear();
    ASSERT_EQ(tmp, e);

    // Capacity is not allowed to shrink after clear().
    ttlet tmp_cap = tmp.capacity();
    ttlet e_cap = e.capacity();
    ASSERT_TRUE(tmp_cap >= start_size);
    ASSERT_TRUE(e_cap >= start_size);

    // Insert to at least two reallocation.
    for (size_t i = 0; i != start_size; ++i) {
        // Get a semi random number indexing into the current gap_buffer and vector, or one beyond.
        auto index = hash_mix_two(i, i) % (i + 1);

        tmp.insert_before(tmp.begin() + index, narrow_cast<int>(i * 3));
        e.insert(e.begin() + index, narrow_cast<int>(i * 3));
        ASSERT_EQ(tmp, e);

        // Capacity is not allowed to grow when inserting data when there is room.
        ASSERT_TRUE(tmp.capacity() == tmp_cap);
        ASSERT_TRUE(e.capacity() == e_cap);
    }
}

TEST(gap_buffer, insert_after_reserve)
{
    ttlet start_size = 500;

    auto tmp = gap_buffer<int>{};
    auto e = std::vector<int>{};
    ASSERT_EQ(tmp, e);

    tmp.reserve(start_size);
    e.reserve(start_size);
    ASSERT_EQ(tmp, e);

    ttlet tmp_cap = tmp.capacity();
    ttlet e_cap = e.capacity();
    ASSERT_TRUE(tmp_cap >= start_size);
    ASSERT_TRUE(e_cap >= start_size);

    // Insert to at least two reallocation.
    for (size_t i = 0; i != start_size; ++i) {
        // Get a semi random number indexing into the current gap_buffer and vector, or one beyond.
        auto index = hash_mix_two(i, i) % (i + 1);

        tmp.insert_before(tmp.begin() + index, narrow_cast<int>(i * 3));
        e.insert(e.begin() + index, narrow_cast<int>(i * 3));
        ASSERT_EQ(tmp, e);

        // Capacity is not allowed to grow when inserting data when there is room.
        ASSERT_TRUE(tmp.capacity() == tmp_cap);
        ASSERT_TRUE(e.capacity() == e_cap);
    }
}
