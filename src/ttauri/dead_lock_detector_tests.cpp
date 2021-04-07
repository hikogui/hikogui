// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/dead_lock_detector.hpp"
#include "ttauri/required.hpp"
#include "ttauri/exception.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;
using namespace tt;

TEST(dead_lock_detector, good)
{
    int a, b;

    ASSERT_NO_THROW(dead_lock_detector::lock(&a));
    ASSERT_NO_THROW(dead_lock_detector::lock(&b));
    ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
    ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);

}

TEST(dead_lock_detector, relock1)
{
    int a, b;

    ASSERT_NO_THROW(dead_lock_detector::lock(&a));
    ASSERT_NO_THROW(dead_lock_detector::lock(&b));
    ASSERT_THROW(dead_lock_detector::lock(&a), lock_error);

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
}

TEST(dead_lock_detector, relock2)
{
    int a, b;

    ASSERT_NO_THROW(dead_lock_detector::lock(&a));
    ASSERT_NO_THROW(dead_lock_detector::lock(&b));
    ASSERT_THROW(dead_lock_detector::lock(&b), lock_error);

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
}

TEST(dead_lock_detector, relock_recursive1)
{
    int a, b;

    ASSERT_NO_THROW(dead_lock_detector::lock(&a, true));
    ASSERT_NO_THROW(dead_lock_detector::lock(&b, true));
    ASSERT_NO_THROW(dead_lock_detector::lock(&b, true));

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
}

TEST(dead_lock_detector, relock_recursive2)
{
    int a, b;

    ASSERT_NO_THROW(dead_lock_detector::lock(&a, true));
    ASSERT_NO_THROW(dead_lock_detector::lock(&b, true));
    ASSERT_THROW(dead_lock_detector::lock(&a, true), lock_error);

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
}

TEST(dead_lock_detector, unlock1)
{
    int a, b;

    ASSERT_THROW(dead_lock_detector::unlock(&a), lock_error);

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
}

TEST(dead_lock_detector, unlock2)
{
    int a, b;

    ASSERT_NO_THROW(dead_lock_detector::lock(&b));
    ASSERT_THROW(dead_lock_detector::unlock(&a), lock_error);

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
}

TEST(dead_lock_detector, unlock_different_thread)
{
    int a;

    auto at = std::thread([&a](){
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
    });
    at.join();

    auto bt = std::thread([&a](){
        ASSERT_THROW(dead_lock_detector::unlock(&a), lock_error);
    });
    bt.join();

    dead_lock_detector::remove_object(&a);
}

TEST(dead_lock_detector, dead_lock1)
{
    int a, b, c;

    auto at = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_THROW(dead_lock_detector::lock(&a), lock_error);
    });
    bt.join();

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
    dead_lock_detector::remove_object(&c);
}

TEST(dead_lock_detector, dead_lock2)
{
    int a, b, c;

    auto at = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_THROW(dead_lock_detector::lock(&b), lock_error);
    });
    bt.join();

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
    dead_lock_detector::remove_object(&c);
}

TEST(dead_lock_detector, dead_lock3)
{
    int a, b, c;

    auto at = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_THROW(dead_lock_detector::lock(&a), lock_error);
    });
    bt.join();

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
    dead_lock_detector::remove_object(&c);
}

TEST(dead_lock_detector, good_lock1)
{
    int a, b, c;

    auto at = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    });
    bt.join();

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
    dead_lock_detector::remove_object(&c);
}

TEST(dead_lock_detector, good_lock2)
{
    int a, b, c;

    auto at = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    });
    bt.join();

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
    dead_lock_detector::remove_object(&c);
}

TEST(dead_lock_detector, good_lock3)
{
    int a, b, c;

    auto at = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
    });
    bt.join();

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
    dead_lock_detector::remove_object(&c);
}

TEST(dead_lock_detector, good_lock4)
{
    int a, b, c;

    auto at = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    });
    at.join();

    auto bt = std::thread([&]() {
        ASSERT_NO_THROW(dead_lock_detector::lock(&a));
        ASSERT_NO_THROW(dead_lock_detector::lock(&b));
        ASSERT_NO_THROW(dead_lock_detector::lock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&c));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&b));
        ASSERT_NO_THROW(dead_lock_detector::unlock(&a));
    });
    bt.join();

    dead_lock_detector::remove_object(&a);
    dead_lock_detector::remove_object(&b);
    dead_lock_detector::remove_object(&c);
}
