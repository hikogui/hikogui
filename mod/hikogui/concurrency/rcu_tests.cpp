// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "rcu.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>

using namespace std;
using namespace hi;

TEST(rcu, read)
{
    auto object = rcu<int>{};
    ASSERT_EQ(object.version(), 0);
    ASSERT_TRUE(object.empty());
    ASSERT_EQ(object.capacity(), 0);
    ASSERT_EQ(object.get(), nullptr);

    object.emplace(42);
    ASSERT_EQ(object.version(), 1);
    ASSERT_FALSE(object.empty());
    ASSERT_EQ(object.capacity(), 1);

    object.lock();
    auto ptr = object.get();
    ASSERT_NE(ptr, nullptr);
    ASSERT_EQ(*ptr, 42);
    ASSERT_EQ(object.version(), 1);

    object.unlock();
    ASSERT_EQ(object.version(), 2);
}

TEST(rcu, write_while_read)
{
    auto object = rcu<int>{};
    ASSERT_EQ(object.version(), 0);
    ASSERT_TRUE(object.empty());
    ASSERT_EQ(object.get(), nullptr);
    ASSERT_EQ(object.capacity(), 0);

    object.emplace(42);
    ASSERT_EQ(object.version(), 1);
    ASSERT_FALSE(object.empty());
    ASSERT_EQ(object.capacity(), 1);

    object.lock();
    auto ptr42 = object.get();
    ASSERT_NE(ptr42, nullptr);
    ASSERT_EQ(*ptr42, 42);
    ASSERT_EQ(object.version(), 1);

    object.emplace(5);
    ASSERT_EQ(object.version(), 1);
    ASSERT_EQ(object.capacity(), 2);

    object.lock();
    auto ptr5 = object.get();
    ASSERT_NE(ptr5, nullptr);
    ASSERT_EQ(*ptr5, 5);
    ASSERT_EQ(object.version(), 1);
    object.unlock();
    // The version does not increment while a lock is being held.
    ASSERT_EQ(object.version(), 1);

    object.unlock();
    ASSERT_EQ(object.version(), 2);
    // The capacity does not change when just reading.
    ASSERT_EQ(object.capacity(), 2);

    // Reset will assign nullptr.
    // At this point there is no lock being held, so old allocations are removed.
    object.reset();
    ASSERT_EQ(object.version(), 3);
    ASSERT_EQ(object.capacity(), 0);
    ASSERT_TRUE(object.empty());
}
