// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "notifier.hpp"
#include "scoped_task.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(notifier, local)
{
    auto a = 0;
    auto b = 0;

    auto n = notifier{};

    auto a_cbt = n.subscribe(
        [&] {
            ++a;
        },
        callback_flags::local);

    auto b_cbt = n.subscribe(
        [&] {
            ++b;
        },
        callback_flags::local);

    // Post the functions to the local event-loop.
    // The two functions are not called immediately, not until the event-loop is resumed.
    n();
    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 0);

    loop::local().resume_once();
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 1);
}

TEST(notifier, local_unsubscribe)
{
    auto a = 0;
    auto b = 0;

    auto n = notifier{};

    auto a_cbt = n.subscribe(
        [&] {
            ++a;
        },
        callback_flags::local);

    auto b_cbt = n.subscribe(
        [&] {
            ++b;
        },
        callback_flags::local);

    // Unsubscribe from a.
    a_cbt = {};

    // Post the callbacks to the local event-loop.
    n();
    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 0);

    loop::local().resume_once();

    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 1);
}

scoped_task<> local_coroutine_func(int& a, int& b, notifier<>& n)
{
    ++a;
    co_await n;
    ++b;
}

TEST(notifier, local_coroutine)
{
    auto a = 0;
    auto b = 0;

    auto n = notifier{};

    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 0);

    // Start a coroutine, the first part of the coroutine is immediatly executed.
    auto cr = local_coroutine_func(a, b, n);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 0);

    // Post the callbacks to the local event-loop.
    // The coroutine will not continue until the event loop is resumed.
    n();
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 0);
    ASSERT_FALSE(cr.done());

    loop::local().resume_once();

    // Now the coroutine has continued and completed.
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 1);
    ASSERT_TRUE(cr.done());
}