// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "callback.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <future>

TEST(callback, call_direct)
{
    int v = 42;

    auto cb = hi::callback<void(int)>([&](int x){ v += x; });

    ASSERT_EQ(v, 42);
    cb(3);
    ASSERT_EQ(v, 45);
}

TEST(callback, call_through_weak)
{
    int v = 42;

    auto cb = hi::callback<void(int)>([&](int x){ v += x; });
    auto wcb = hi::weak_callback<void(int)>{cb};

    ASSERT_EQ(v, 42);
    if (wcb.lock()) {
        wcb(3);
        wcb.unlock();
    }
    ASSERT_EQ(v, 45);
}

TEST(callback, delay_destruction)
{
    using namespace std::literals;

    std::atomic<int> state = 0;
    int v = 42;

    auto cb = hi::callback<void(int)>([&](int x){ v += x; });
    auto wcb = hi::weak_callback<void(int)>{cb};

    // The callback object is still alive, we can still lock.
    ASSERT_FALSE(wcb.expired());
    ASSERT_TRUE(wcb.lock());
    
    // Attempt the callback object.
    auto future = std::async(std::launch::async, [&] {
        state = 1;
        cb = nullptr;
        // The destruction is being delayed until everything is unlocked.
        state = 2;
    });

    // Wait until the other thread has started destroying the callback.
    while (state == 0) {
        std::this_thread::sleep_for(25ms);
    }
    ASSERT_EQ(state, 1);

    // The callback object is being destroyed, we can no longer lock.
    ASSERT_TRUE(wcb.expired());
    ASSERT_FALSE(wcb.lock());

    ASSERT_EQ(v, 42);
    wcb(3);
    ASSERT_EQ(v, 45);

    // Unlock the weak_callback after which the other thread can finish
    // with the destruction of the callback.
    ASSERT_EQ(state, 1);
    wcb.unlock();

    // Wait until the other thread has finished destroying the callback.
    while (state == 1) {
        std::this_thread::sleep_for(25ms);
    }
    ASSERT_EQ(state, 2);
}
