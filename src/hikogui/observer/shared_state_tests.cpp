// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "shared_state.hpp"
#include "observer_intf.hpp"
#include "../dispatch/dispatch.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

namespace test_shared_space {

struct B {
    std::string foo;
    int bar;

    [[nodiscard]] friend bool operator==(B const&, B const&) noexcept = default;
};

struct A {
    B b;
    std::vector<int> baz;

    [[nodiscard]] friend bool operator==(A const&, A const&) noexcept = default;
};

} // namespace test_shared_space

// clang-format off
template<>
struct hi::selector<test_shared_space::B> {
    template<hi::fixed_string> [[nodiscard]] auto &get(test_shared_space::B &rhs) const noexcept;

    template<> [[nodiscard]] auto &get<"foo">(test_shared_space::B &rhs) const noexcept { return rhs.foo; }
    template<> [[nodiscard]] auto &get<"bar">(test_shared_space::B &rhs) const noexcept { return rhs.bar; }
};

template<>
struct hi::selector<test_shared_space::A> {
    template<hi::fixed_string> [[nodiscard]] auto &get(test_shared_space::A &rhs) const noexcept;

    template<> [[nodiscard]] auto &get<"b">(test_shared_space::A &rhs) const noexcept { return rhs.b; }
    template<> [[nodiscard]] auto &get<"baz">(test_shared_space::A &rhs) const noexcept { return rhs.baz; }
};
// clang-format on

TEST(shared_state, read)
{
    using namespace test_shared_space;

    auto state = hi::shared_state<A>{B{"hello world", 42}, std::vector<int>{5, 15}};

    auto a_cursor = state.observer();
    auto baz_cursor = state.sub<"baz">();
    auto baz0_cursor = state.sub<"baz">().sub(0);
    auto baz1_cursor = baz_cursor.sub(1);
    auto b_cursor = a_cursor.sub<"b">();
    auto foo_cursor = state.sub<"b">().sub<"foo">();
    auto bar_cursor = b_cursor.sub<"bar">();

    ASSERT_EQ(foo_cursor, "hello world");
    ASSERT_EQ(bar_cursor, 42);
    auto baz_result = std::vector<int>{5, 15};
    ASSERT_EQ(baz_cursor, baz_result);
    ASSERT_EQ(baz0_cursor, 5);
    ASSERT_EQ(baz1_cursor, 15);

    auto b_proxy = b_cursor.get();
    ASSERT_EQ(b_proxy->foo, "hello world");
    ASSERT_EQ(b_cursor->bar, 42);

    auto a_proxy = a_cursor.get();
    ASSERT_EQ(a_proxy->b.foo, "hello world");
    ASSERT_EQ(a_proxy->b.bar, 42);
    ASSERT_EQ(a_proxy->baz, baz_result);
}

TEST(shared_state, notify)
{
    using namespace test_shared_space;

    auto state = hi::shared_state<A>{B{"hello world", 42}, std::vector<int>{5, 15}};

    auto a_cursor = state.observer();
    auto b_cursor = a_cursor.sub<"b">();
    auto foo_cursor = b_cursor.sub<"foo">();
    auto bar_cursor = b_cursor.sub<"bar">();
    auto barD_cursor = b_cursor.sub<"bar">();
    auto baz_cursor = a_cursor.sub<"baz">();
    auto baz0_cursor = baz_cursor.sub(0);
    auto baz1_cursor = baz_cursor.sub(1);

    auto a_count = 0;
    auto b_count = 0;
    auto foo_count = 0;
    auto bar_count = 0;
    auto barD_count = 0;
    auto baz_count = 0;
    auto baz0_count = 0;
    auto baz1_count = 0;

    // clang-format off
    auto a_cbt = a_cursor.subscribe([&](auto...) { ++a_count; });
    auto b_cbt = b_cursor.subscribe([&](auto...) { ++b_count; });
    auto foo_cbt = foo_cursor.subscribe([&](auto...) { ++foo_count; });
    auto bar_cbt = bar_cursor.subscribe([&](auto...) { ++bar_count; });
    auto barD_cbt = barD_cursor.subscribe([&](auto...) { ++barD_count; });
    auto baz_cbt = baz_cursor.subscribe([&](auto...) { ++baz_count; });
    auto baz0_cbt = baz0_cursor.subscribe([&](auto...) { ++baz0_count; });
    auto baz1_cbt = baz1_cursor.subscribe([&](auto...) { ++baz1_count; });
    // clang-format on

    {
        a_count = 0;
        b_count = 0;
        foo_count = 0;
        bar_count = 0;
        barD_count = 0;
        baz_count = 0;
        baz0_count = 0;
        baz1_count = 0;
        a_cursor->b.bar = 3;
        ASSERT_EQ(a_cursor->b.bar, 3);
        ASSERT_EQ(a_count, 1);
        ASSERT_EQ(b_count, 1);
        ASSERT_EQ(foo_count, 1);
        ASSERT_EQ(bar_count, 1);
        ASSERT_EQ(barD_count, 1);
        ASSERT_EQ(baz_count, 1);
        ASSERT_EQ(baz0_count, 1);
        ASSERT_EQ(baz1_count, 1);
    }

    {
        a_count = 0;
        b_count = 0;
        foo_count = 0;
        bar_count = 0;
        barD_count = 0;
        baz_count = 0;
        baz0_count = 0;
        baz1_count = 0;
        b_cursor->bar = 5;
        ASSERT_EQ(a_cursor->b.bar, 5);
        ASSERT_EQ(a_count, 1);
        ASSERT_EQ(b_count, 1);
        ASSERT_EQ(foo_count, 1);
        ASSERT_EQ(bar_count, 1);
        ASSERT_EQ(barD_count, 1);
        ASSERT_EQ(baz_count, 0);
        ASSERT_EQ(baz0_count, 0);
        ASSERT_EQ(baz1_count, 0);
    }

    {
        a_count = 0;
        b_count = 0;
        foo_count = 0;
        bar_count = 0;
        barD_count = 0;
        baz_count = 0;
        baz0_count = 0;
        baz1_count = 0;
        bar_cursor = 7;
        ASSERT_EQ(a_cursor->b.bar, 7);
        ASSERT_EQ(a_count, 1);
        ASSERT_EQ(b_count, 1);
        ASSERT_EQ(foo_count, 0);
        ASSERT_EQ(bar_count, 1);
        ASSERT_EQ(barD_count, 1);
        ASSERT_EQ(baz_count, 0);
        ASSERT_EQ(baz0_count, 0);
        ASSERT_EQ(baz1_count, 0);
    }

    {
        a_count = 0;
        b_count = 0;
        foo_count = 0;
        bar_count = 0;
        barD_count = 0;
        baz_count = 0;
        baz0_count = 0;
        baz1_count = 0;
        baz_cursor->push_back(7);
        auto baz_result = std::vector<int>{5, 15, 7};
        ASSERT_EQ(a_cursor->baz, baz_result);
        ASSERT_EQ(a_count, 1);
        ASSERT_EQ(b_count, 0);
        ASSERT_EQ(foo_count, 0);
        ASSERT_EQ(bar_count, 0);
        ASSERT_EQ(barD_count, 0);
        ASSERT_EQ(baz_count, 1);
        ASSERT_EQ(baz0_count, 1);
        ASSERT_EQ(baz1_count, 1);
    }

    {
        a_count = 0;
        b_count = 0;
        foo_count = 0;
        bar_count = 0;
        barD_count = 0;
        baz_count = 0;
        baz0_count = 0;
        baz1_count = 0;
        baz0_cursor = 1;
        ASSERT_EQ(a_cursor->baz[0], 1);
        ASSERT_EQ(a_count, 1);
        ASSERT_EQ(b_count, 0);
        ASSERT_EQ(foo_count, 0);
        ASSERT_EQ(bar_count, 0);
        ASSERT_EQ(barD_count, 0);
        ASSERT_EQ(baz_count, 1);
        ASSERT_EQ(baz0_count, 1);
        ASSERT_EQ(baz1_count, 0);
    }
}


TEST(shared_state, value)
{
    using namespace test_shared_space;

    bool a_modified = false;

    hi::observer<int> a;
    auto a_cbt = a.subscribe([&a_modified](auto...) {
        a_modified = true;
    });
    ASSERT_FALSE(a_modified);
    ASSERT_EQ(*a, 0);
    a_modified = false;

    a = 1;
    ASSERT_TRUE(a_modified);
    ASSERT_EQ(*a, 1);
    a_modified = false;
}

TEST(shared_state, chain1)
{
    using namespace test_shared_space;

    bool a_modified = false;
    bool b_modified = false;

    hi::observer<int> a;
    hi::observer<int> b;
    auto a_cbt = a.subscribe([&a_modified](auto...) {
        a_modified = true;
    });
    auto b_cbt = b.subscribe([&b_modified](auto...) {
        b_modified = true;
    });

    ASSERT_FALSE(a_modified);
    ASSERT_FALSE(b_modified);
    ASSERT_EQ(*a, 0);
    ASSERT_EQ(*b, 0);
    a_modified = false;
    b_modified = false;

    a = 1;
    b = 2;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_EQ(*a, 1);
    ASSERT_EQ(*b, 2);
    a_modified = false;
    b_modified = false;

    a = b;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_EQ(*a, 2);
    ASSERT_EQ(*b, 2);
    a_modified = false;

    b = 3;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_EQ(*a, 3);
    ASSERT_EQ(*b, 3);
    a_modified = false;
    b_modified = false;

    a = 4;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_EQ(*a, 4);
    ASSERT_EQ(*b, 4);
    a_modified = false;
    b_modified = false;
}

TEST(shared_state, chain2)
{
    using namespace test_shared_space;

    bool a_modified = false;
    bool b_modified = false;
    bool c_modified = false;

    hi::observer<int> a;
    hi::observer<int> b;
    hi::observer<int> c;

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
    ASSERT_EQ(*a, 0);
    ASSERT_EQ(*b, 0);
    ASSERT_EQ(*c, 0);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 1;
    b = 2;
    c = 3;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 1);
    ASSERT_EQ(*b, 2);
    ASSERT_EQ(*c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = b;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_FALSE(c_modified);
    ASSERT_EQ(*a, 2);
    ASSERT_EQ(*b, 2);
    ASSERT_EQ(*c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = c;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 3);
    ASSERT_EQ(*b, 3);
    ASSERT_EQ(*c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    c = 4;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 4);
    ASSERT_EQ(*b, 4);
    ASSERT_EQ(*c, 4);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = 5;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 5);
    ASSERT_EQ(*b, 5);
    ASSERT_EQ(*c, 5);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 6;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 6);
    ASSERT_EQ(*b, 6);
    ASSERT_EQ(*c, 6);
    a_modified = false;
    b_modified = false;
    c_modified = false;
}

TEST(shared_state, chain3)
{
    using namespace test_shared_space;

    bool a_modified = false;
    bool b_modified = false;
    bool c_modified = false;

    hi::observer<int> a;
    hi::observer<int> b;
    hi::observer<int> c;

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
    ASSERT_EQ(*a, 0);
    ASSERT_EQ(*b, 0);
    ASSERT_EQ(*c, 0);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 1;
    b = 2;
    c = 3;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 1);
    ASSERT_EQ(*b, 2);
    ASSERT_EQ(*c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = c;
    ASSERT_FALSE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 1);
    ASSERT_EQ(*b, 3);
    ASSERT_EQ(*c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = b;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 3);
    ASSERT_EQ(*b, 3);
    ASSERT_EQ(*c, 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    c = 4;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 4);
    ASSERT_EQ(*b, 4);
    ASSERT_EQ(*c, 4);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = 5;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 5);
    ASSERT_EQ(*b, 5);
    ASSERT_EQ(*c, 5);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 6;
    ASSERT_TRUE(a_modified);
    ASSERT_TRUE(b_modified);
    ASSERT_TRUE(c_modified);
    ASSERT_EQ(*a, 6);
    ASSERT_EQ(*b, 6);
    ASSERT_EQ(*c, 6);
    a_modified = false;
    b_modified = false;
    c_modified = false;
}

void callback1(int new_value)
{
    ASSERT_EQ(new_value, 42);
}

void callback2(int const& new_value)
{
    ASSERT_EQ(new_value, 42);
}

TEST(shared_state, callback)
{
    using namespace test_shared_space;

    auto a = hi::observer<int>{1};

    // This tests if we can both subscribe a callback that accepts the
    // argument by value or by const reference.
    auto cbt1 = a.subscribe(callback1);
    auto cbt2 = a.subscribe(callback2);

    a = 42;
}

TEST(shared_state, convenience_operators)
{
    auto a = hi::observer<int>{};
    ASSERT_EQ(a, 0);

    a = 1;
    ASSERT_EQ(a, 1);

    a += 2;
    ASSERT_EQ(a, 3);
}
