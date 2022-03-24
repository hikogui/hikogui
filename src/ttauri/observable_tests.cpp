// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "observable.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(observable, value)
{
    bool a_modified = false;

    observable<int> a;
    auto a_cbt = a.subscribe([&a_modified](auto...) {
        a_modified = true;
    });
    ASSERT_FALSE(a_modified);
    ASSERT_EQ(a, 0);
    a_modified = false;

    a = 1;
    ASSERT_TRUE(a_modified);
    ASSERT_EQ(a, 1);
    a_modified = false;
}

TEST(observable, chain1)
{
    bool a_modified = false;
    bool b_modified = false;

    observable<int> a;
    observable<int> b;
    auto a_cbt = a.subscribe([&a_modified](auto...) {
        a_modified = true;
    });
    auto b_cbt = b.subscribe([&b_modified](auto...) {
        b_modified = true;
    });

    ASSERT_FALSE(a_modified);
    ASSERT_FALSE(b_modified);
    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 0);
    a_modified = false;
    b_modified = false;

    a = 1;
    b = 2;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 2);
    a_modified = false;
    b_modified = false;

    a = b;
    ASSERT_TRUE(a_modified);
    ASSERT_FALSE(b_modified);
    ASSERT_EQ(a, 2);
    ASSERT_EQ(b, 2);
    a_modified = false;

    b = 3;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_EQ(a, 3);
    ASSERT_EQ(b, 3);
    a_modified = false;
    b_modified = false;

    a = 4;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_EQ(a, 4);
    ASSERT_EQ(b, 4);
    a_modified = false;
    b_modified = false;
}

TEST(observable, chain2)
{
    bool a_modified = false;
    bool b_modified = false;
    bool c_modified = false;

    observable<int> a;
    observable<int> b;
    observable<int> c;

    auto a_cbt = a.subscribe([&a_modified](auto...) {
        a_modified = true;
    });
    auto b_cbt = b.subscribe([&b_modified](auto...) {
        b_modified = true;
    });
    auto c_cbt = c.subscribe([&c_modified](auto...) {
        c_modified = true;
    });

    ASSERT_FALSE(a_modified);
    ASSERT_FALSE(b_modified);
    ASSERT_FALSE(c_modified);
    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 0);
    ASSERT_EQ(c, 0);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 1;
    b = 2;
    c = 3;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 2);
    ASSERT_EQ(c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = b;
    ASSERT_TRUE(a_modified);
    ASSERT_FALSE(b_modified);
    ASSERT_FALSE(c_modified);
    ASSERT_EQ(a, 2);
    ASSERT_EQ(b, 2);
    ASSERT_EQ(c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = c;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_FALSE(c_modified);
    ASSERT_EQ(a, 3);
    ASSERT_EQ(b, 3);
    ASSERT_EQ(c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    c = 4;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(a, 4);
    ASSERT_EQ(b, 4);
    ASSERT_EQ(c, 4);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = 5;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(a, 5);
    ASSERT_EQ(b, 5);
    ASSERT_EQ(c, 5);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 6;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(a, 6);
    ASSERT_EQ(b, 6);
    ASSERT_EQ(c, 6);
    a_modified = false;
    b_modified = false;
    c_modified = false;
}

TEST(observable, chain3)
{
    bool a_modified = false;
    bool b_modified = false;
    bool c_modified = false;

    observable<int> a;
    observable<int> b;
    observable<int> c;

    auto a_cbt = a.subscribe([&a_modified](auto...) {
        a_modified = true;
    });
    auto b_cbt = b.subscribe([&b_modified](auto...) {
        b_modified = true;
    });
    auto c_cbt = c.subscribe([&c_modified](auto...) {
        c_modified = true;
    });

    ASSERT_FALSE(a_modified);
    ASSERT_FALSE(b_modified);
    ASSERT_FALSE(c_modified);
    ASSERT_EQ(a, 0);
    ASSERT_EQ(b, 0);
    ASSERT_EQ(c, 0);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 1;
    b = 2;
    c = 3;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 2);
    ASSERT_EQ(c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = c;
    ASSERT_FALSE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_FALSE(c_modified);
    ASSERT_EQ(a, 1);
    ASSERT_EQ(b, 3);
    ASSERT_EQ(c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = b;
    ASSERT_TRUE(a_modified);
    ASSERT_FALSE(b_modified);
    ASSERT_FALSE(c_modified);
    ASSERT_EQ(a, 3);
    ASSERT_EQ(b, 3);
    ASSERT_EQ(c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    c = 4;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(a, 4);
    ASSERT_EQ(b, 4);
    ASSERT_EQ(c, 4);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = 5;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(a, 5);
    ASSERT_EQ(b, 5);
    ASSERT_EQ(c, 5);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 6;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(a, 6);
    ASSERT_EQ(b, 6);
    ASSERT_EQ(c, 6);
    a_modified = false;
    b_modified = false;
    c_modified = false;
}

void callback1(int value)
{
    ASSERT_EQ(value, 42);
}

void callback2(int const& value)
{
    ASSERT_EQ(value, 42);
}

TEST(observable, callback)
{
    observable<int> a;

    // This tests if we can both subscribe a callback that accepts the
    // argument by value or by const reference.
    auto cbt1 = a.subscribe(callback1);
    auto cbt2 = a.subscribe(callback2);

    a = 42;
}
