// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unfair_mutex.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <hikotest/hikotest.hpp>
#include <iostream>
#include <string>
#include <thread>

TEST_SUITE(dead_lock_detector_suite)
{

TEST_CASE(good_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b;

    REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
    REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
    REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
    REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
}

TEST_CASE(relock1_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b;

    REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
    REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
    REQUIRE(hi::unfair_mutex_deadlock_lock(&a) != nullptr);

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
}

TEST_CASE(relock2_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b;

    REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
    REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
    REQUIRE(hi::unfair_mutex_deadlock_lock(&b) != nullptr);

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
}

TEST_CASE(unlock1_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b;

    REQUIRE(not hi::unfair_mutex_deadlock_unlock(&a));

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
}

TEST_CASE(unlock2_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b;

    REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
    REQUIRE(not hi::unfair_mutex_deadlock_unlock(&a));

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
}

TEST_CASE(unlock_different_thread_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a;

    auto at = std::thread([&a]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
    });
    at.join();

    auto bt = std::thread([&a]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(not hi::unfair_mutex_deadlock_unlock(&a));
    });
    bt.join();

    hi::unfair_mutex_deadlock_remove_object(&a);
}

TEST_CASE(dead_lock1_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&c));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) != nullptr);
    });
    bt.join();

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
    hi::unfair_mutex_deadlock_remove_object(&c);
}

TEST_CASE(dead_lock2_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&c));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) != nullptr);
    });
    bt.join();

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
    hi::unfair_mutex_deadlock_remove_object(&c);
}

TEST_CASE(dead_lock3_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&c));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) != nullptr);
    });
    bt.join();

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
    hi::unfair_mutex_deadlock_remove_object(&c);
}

TEST_CASE(good_lock1_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&c));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    });
    bt.join();

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
    hi::unfair_mutex_deadlock_remove_object(&c);
}

TEST_CASE(good_lock2_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&c));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&c));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    });
    bt.join();

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
    hi::unfair_mutex_deadlock_remove_object(&c);
}

TEST_CASE(good_lock3_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&c));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&c));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
    });
    bt.join();

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
    hi::unfair_mutex_deadlock_remove_object(&c);
}

TEST_CASE(good_lock4_test)
{
    hi::unfair_mutex_deadlock_clear_stack();
    hi::unfair_mutex_deadlock_clear_graph();

    int a, b, c;

    auto at = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&c));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        hi::unfair_mutex_deadlock_clear_stack();
        REQUIRE(hi::unfair_mutex_deadlock_lock(&a) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&b) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_lock(&c) == nullptr);
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&c));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&b));
        REQUIRE(hi::unfair_mutex_deadlock_unlock(&a));
    });
    bt.join();

    hi::unfair_mutex_deadlock_remove_object(&a);
    hi::unfair_mutex_deadlock_remove_object(&b);
    hi::unfair_mutex_deadlock_remove_object(&c);
}

};
