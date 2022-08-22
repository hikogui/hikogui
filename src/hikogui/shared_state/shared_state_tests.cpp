// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "shared_state.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

struct B {
    std::string foo;
    int bar;
};

struct A {
    B b;
    std::vector<int> baz;
};

// clang-format off
template<>
struct hi::selector<B> {
    template<hi::basic_fixed_string> [[nodiscard]] auto &get(B &rhs) const noexcept;

    template<> [[nodiscard]] auto &get<"foo">(B &rhs) const noexcept { return rhs.foo; }
    template<> [[nodiscard]] auto &get<"bar">(B &rhs) const noexcept { return rhs.bar; }
};

template<>
struct hi::selector<A> {
    template<hi::basic_fixed_string> [[nodiscard]] auto &get(A &rhs) const noexcept;

    template<> [[nodiscard]] auto &get<"b">(A &rhs) const noexcept { return rhs.b; }
    template<> [[nodiscard]] auto &get<"baz">(A &rhs) const noexcept { return rhs.baz; }
};
// clang-format on

TEST(shared_state, read)
{
    auto state = hi::shared_state<A>{B{"hello world", 42}, std::vector<int>{5, 15}};

    auto a_cursor = state.cursor();
    auto baz_cursor = state._<"baz">();
    auto baz0_cursor = state._<"baz">()[0];
    auto baz1_cursor = baz_cursor[1];
    auto b_cursor = a_cursor._<"b">();
    auto foo_cursor = state._<"b">()._<"foo">();
    auto bar_cursor = b_cursor._<"bar">();

    ASSERT_EQ(*foo_cursor.read(), "hello world");
    ASSERT_EQ(*bar_cursor.read(), 42);
    auto baz_result = std::vector<int>{5, 15};
    ASSERT_EQ(*baz_cursor.read(), baz_result);
    ASSERT_EQ(*baz0_cursor.read(), 5);
    ASSERT_EQ(*baz1_cursor.read(), 15);

    auto b_proxy = b_cursor.read();
    ASSERT_EQ(b_proxy->foo, "hello world");
    ASSERT_EQ(b_cursor.read()->bar, 42);

    auto a_proxy = a_cursor.read();
    ASSERT_EQ(a_proxy->b.foo, "hello world");
    ASSERT_EQ(a_proxy->b.bar, 42);
    ASSERT_EQ(a_proxy->baz, baz_result);
}

TEST(shared_state, notify)
{
    auto state = hi::shared_state<A>{B{"hello world", 42}, std::vector<int>{5, 15}};

    auto a_cursor = state.cursor();
    auto b_cursor = a_cursor._<"b">();
    auto foo_cursor = b_cursor._<"foo">();
    auto bar_cursor = b_cursor._<"bar">();
    auto barD_cursor = b_cursor._<"bar">();
    auto baz_cursor = a_cursor._<"baz">();
    auto baz0_cursor = baz_cursor[0];
    auto baz1_cursor = baz_cursor[1];

    auto a_count = 0;
    auto b_count = 0;
    auto foo_count = 0;
    auto bar_count = 0;
    auto barD_count = 0;
    auto baz_count = 0;
    auto baz0_count = 0;
    auto baz1_count = 0;

    // clang-format off
    auto a_cbt = a_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++a_count; });
    auto b_cbt = b_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++b_count; });
    auto foo_cbt = foo_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++foo_count; });
    auto bar_cbt = bar_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++bar_count; });
    auto barD_cbt = barD_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++barD_count; });
    auto baz_cbt = baz_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++baz_count; });
    auto baz0_cbt = baz0_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++baz0_count; });
    auto baz1_cbt = baz1_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++baz1_count; });
    // clang-format on

    a_cursor.copy()->b.bar = 3;
    ASSERT_EQ(a_cursor.read()->b.bar, 3);
    ASSERT_EQ(a_count, 1);
    ASSERT_EQ(b_count, 1);
    ASSERT_EQ(foo_count, 1);
    ASSERT_EQ(bar_count, 1);
    ASSERT_EQ(barD_count, 1);
    ASSERT_EQ(baz_count, 1);
    ASSERT_EQ(baz0_count, 1);
    ASSERT_EQ(baz1_count, 1);

    b_cursor.copy()->bar = 5;
    ASSERT_EQ(a_cursor.read()->b.bar, 5);
    ASSERT_EQ(a_count, 1);
    ASSERT_EQ(b_count, 2);
    ASSERT_EQ(foo_count, 2);
    ASSERT_EQ(bar_count, 2);
    ASSERT_EQ(barD_count, 2);
    ASSERT_EQ(baz_count, 1);
    ASSERT_EQ(baz0_count, 1);
    ASSERT_EQ(baz1_count, 1);

    *bar_cursor.copy() = 7;
    ASSERT_EQ(a_cursor.read()->b.bar, 7);
    ASSERT_EQ(a_count, 1);
    ASSERT_EQ(b_count, 2);
    ASSERT_EQ(foo_count, 2);
    ASSERT_EQ(bar_count, 3);
    ASSERT_EQ(barD_count, 3);
    ASSERT_EQ(baz_count, 1);
    ASSERT_EQ(baz0_count, 1);
    ASSERT_EQ(baz1_count, 1);

    baz_cursor.copy()->push_back(7);
    auto baz_result = std::vector<int>{5, 15, 7};
    ASSERT_EQ(a_cursor.read()->baz, baz_result);
    ASSERT_EQ(a_count, 1);
    ASSERT_EQ(b_count, 2);
    ASSERT_EQ(foo_count, 2);
    ASSERT_EQ(bar_count, 3);
    ASSERT_EQ(barD_count, 3);
    ASSERT_EQ(baz_count, 2);
    ASSERT_EQ(baz0_count, 2);
    ASSERT_EQ(baz1_count, 2);

    *baz0_cursor.copy() = 1;
    ASSERT_EQ(a_cursor.read()->baz[0], 1);
    ASSERT_EQ(a_count, 1);
    ASSERT_EQ(b_count, 2);
    ASSERT_EQ(foo_count, 2);
    ASSERT_EQ(bar_count, 3);
    ASSERT_EQ(barD_count, 3);
    ASSERT_EQ(baz_count, 2);
    ASSERT_EQ(baz0_count, 3);
    ASSERT_EQ(baz1_count, 2);
}

TEST(shared_state, commit_abort)
{
    auto state = hi::shared_state<A>{B{"hello world", 42}, std::vector<int>{5, 15}};

    auto a_cursor = state.cursor();
    auto b_cursor = a_cursor._<"b">();
    auto foo_cursor = b_cursor._<"foo">();

    auto a_count = 0;
    auto b_count = 0;
    auto foo_count = 0;

    // clang-format off
    auto a_cbt = a_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++a_count; });
    auto b_cbt = b_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++b_count; });
    auto foo_cbt = foo_cursor.subscribe(hi::callback_flags::synchronous, [&] { ++foo_count; });
    // clang-format on

    // Commit on end-of-scope.
    {
        auto foo_proxy = foo_cursor.copy();
        *foo_proxy = "1";
        ASSERT_EQ(*foo_cursor.read(), "hello world");
    }
    ASSERT_EQ(*foo_cursor.read(), "1");
    ASSERT_EQ(a_count, 0);
    ASSERT_EQ(b_count, 0);
    ASSERT_EQ(foo_count, 1);

    // Early commit.
    {
        auto foo_proxy = foo_cursor.copy();
        *foo_proxy = "2";
        ASSERT_EQ(*foo_cursor.read(), "1");
        foo_proxy.commit();
        ASSERT_EQ(*foo_cursor.read(), "2");
    }
    ASSERT_EQ(a_count, 0);
    ASSERT_EQ(b_count, 0);
    ASSERT_EQ(foo_count, 2);

    // Early abort.
    {
        auto foo_proxy = foo_cursor.copy();
        *foo_proxy = "3";
        ASSERT_EQ(*foo_cursor.read(), "2");
        foo_proxy.abort();
        ASSERT_EQ(*foo_cursor.read(), "2");
    }
    ASSERT_EQ(a_count, 0);
    ASSERT_EQ(b_count, 0);
    ASSERT_EQ(foo_count, 2);
}