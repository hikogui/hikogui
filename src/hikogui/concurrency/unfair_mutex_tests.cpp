// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unfair_mutex.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;
using namespace hi;

#define ASSERT_NULL(x) ASSERT_EQ(x, nullptr);
#define ASSERT_NOT_NULL(x) ASSERT_NE(x, nullptr);
TEST(dead_lock_detector, good)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b;

    ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
    ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
    ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
    ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
}

TEST(dead_lock_detector, relock1)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b;

    ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
    ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
    ASSERT_NOT_NULL(unfair_mutex_deadlock_lock(&a));

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
}

TEST(dead_lock_detector, relock2)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b;

    ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
    ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
    ASSERT_NOT_NULL(unfair_mutex_deadlock_lock(&b));

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
}

TEST(dead_lock_detector, unlock1)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b;

    ASSERT_FALSE(unfair_mutex_deadlock_unlock(&a));

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
}

TEST(dead_lock_detector, unlock2)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b;

    ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
    ASSERT_FALSE(unfair_mutex_deadlock_unlock(&a));

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
}

TEST(dead_lock_detector, unlock_different_thread)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a;

    auto at = std::thread([&a]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
    });
    at.join();

    auto bt = std::thread([&a]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_FALSE(unfair_mutex_deadlock_unlock(&a));
    });
    bt.join();

    unfair_mutex_deadlock_remove_object(&a);
}

TEST(dead_lock_detector, dead_lock1)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_NOT_NULL(unfair_mutex_deadlock_lock(&a));
    });
    bt.join();

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
    unfair_mutex_deadlock_remove_object(&c);
}

TEST(dead_lock_detector, dead_lock2)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_NOT_NULL(unfair_mutex_deadlock_lock(&b));
    });
    bt.join();

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
    unfair_mutex_deadlock_remove_object(&c);
}

TEST(dead_lock_detector, dead_lock3)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_NOT_NULL(unfair_mutex_deadlock_lock(&a));
    });
    bt.join();

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
    unfair_mutex_deadlock_remove_object(&c);
}

TEST(dead_lock_detector, good_lock1)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    });
    bt.join();

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
    unfair_mutex_deadlock_remove_object(&c);
}

TEST(dead_lock_detector, good_lock2)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    });
    bt.join();

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
    unfair_mutex_deadlock_remove_object(&c);
}

TEST(dead_lock_detector, good_lock3)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
    });
    bt.join();

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
    unfair_mutex_deadlock_remove_object(&c);
}

TEST(dead_lock_detector, good_lock4)
{
    unfair_mutex_deadlock_clear_stack();
    unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        unfair_mutex_deadlock_clear_stack();
        ASSERT_NULL(unfair_mutex_deadlock_lock(&a));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&b));
        ASSERT_NULL(unfair_mutex_deadlock_lock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&c));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&b));
        ASSERT_TRUE(unfair_mutex_deadlock_unlock(&a));
    });
    bt.join();

    unfair_mutex_deadlock_remove_object(&a);
    unfair_mutex_deadlock_remove_object(&b);
    unfair_mutex_deadlock_remove_object(&c);
}
