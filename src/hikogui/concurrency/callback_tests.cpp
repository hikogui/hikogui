// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "callback.hpp"
#include "../macros.hpp"
#include <hikotest/hikotest.hpp>
#include <future>

TEST_SUITE(callback_suite)
{

TEST_CASE(call_direct_test)
{
    int v = 42;

    auto cb = hi::callback<void(int)>([&](int x){ v += x; });

    REQUIRE(v == 42);
    cb(3);
    REQUIRE(v == 45);
}

TEST_CASE(call_through_weak_test)
{
    int v = 42;

    auto cb = hi::callback<void(int)>([&](int x){ v += x; });
    auto wcb = hi::weak_callback<void(int)>{cb};

    REQUIRE(v == 42);
    if (auto tmp = wcb.lock()) {
        tmp(3);
    }
    REQUIRE(v == 45);
}

};
