// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "shared_state.hpp"
#include "observer_intf.hpp"
#include "../dispatch/dispatch.hpp"
#include <hikotest/hikotest.hpp>

namespace shared_state_suite_ns {

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

} // namespace shared_state_suite_ns

// clang-format off
template<>
struct hi::selector<shared_state_suite_ns::B> {
    template<hi::fixed_string> [[nodiscard]] auto &get(shared_state_suite_ns::B &rhs) const noexcept;

    template<> [[nodiscard]] auto &get<"foo">(shared_state_suite_ns::B &rhs) const noexcept { return rhs.foo; }
    template<> [[nodiscard]] auto &get<"bar">(shared_state_suite_ns::B &rhs) const noexcept { return rhs.bar; }
};

template<>
struct hi::selector<shared_state_suite_ns::A> {
    template<hi::fixed_string> [[nodiscard]] auto &get(shared_state_suite_ns::A &rhs) const noexcept;

    template<> [[nodiscard]] auto &get<"b">(shared_state_suite_ns::A &rhs) const noexcept { return rhs.b; }
    template<> [[nodiscard]] auto &get<"baz">(shared_state_suite_ns::A &rhs) const noexcept { return rhs.baz; }
};
// clang-format on

TEST_SUITE(shared_state_suite) {

TEST_CASE(read)
{
    using namespace shared_state_suite_ns;

    auto state = hi::shared_state<A>{B{"hello world", 42}, std::vector<int>{5, 15}};

    auto a_cursor = state.observer();
    auto baz_cursor = state.sub<"baz">();
    auto baz0_cursor = state.sub<"baz">().sub(0);
    auto baz1_cursor = baz_cursor.sub(1);
    auto b_cursor = a_cursor.sub<"b">();
    auto foo_cursor = state.sub<"b">().sub<"foo">();
    auto bar_cursor = b_cursor.sub<"bar">();

    REQUIRE((foo_cursor == "hello world"));
    REQUIRE((bar_cursor == 42));
    auto baz_result = std::vector<int>{5, 15};
    REQUIRE((baz_cursor == baz_result));
    REQUIRE((baz0_cursor == 5));
    REQUIRE((baz1_cursor == 15));

    auto b_proxy = b_cursor.get();
    REQUIRE(b_proxy->foo == "hello world");
    REQUIRE(b_cursor->bar == 42);

    auto a_proxy = a_cursor.get();
    REQUIRE(a_proxy->b.foo == "hello world");
    REQUIRE(a_proxy->b.bar == 42);
    REQUIRE(a_proxy->baz == baz_result);
}

TEST_CASE(notify)
{
    using namespace shared_state_suite_ns;

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
        REQUIRE(a_cursor->b.bar == 3);
        REQUIRE(a_count == 1);
        REQUIRE(b_count == 1);
        REQUIRE(foo_count == 1);
        REQUIRE(bar_count == 1);
        REQUIRE(barD_count == 1);
        REQUIRE(baz_count == 1);
        REQUIRE(baz0_count == 1);
        REQUIRE(baz1_count == 1);
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
        REQUIRE(a_cursor->b.bar == 5);
        REQUIRE(a_count == 1);
        REQUIRE(b_count == 1);
        REQUIRE(foo_count == 1);
        REQUIRE(bar_count == 1);
        REQUIRE(barD_count == 1);
        REQUIRE(baz_count == 0);
        REQUIRE(baz0_count == 0);
        REQUIRE(baz1_count == 0);
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
        REQUIRE(a_cursor->b.bar == 7);
        REQUIRE(a_count == 1);
        REQUIRE(b_count == 1);
        REQUIRE(foo_count == 0);
        REQUIRE(bar_count == 1);
        REQUIRE(barD_count == 1);
        REQUIRE(baz_count == 0);
        REQUIRE(baz0_count == 0);
        REQUIRE(baz1_count == 0);
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
        REQUIRE(a_cursor->baz == baz_result);
        REQUIRE(a_count == 1);
        REQUIRE(b_count == 0);
        REQUIRE(foo_count == 0);
        REQUIRE(bar_count == 0);
        REQUIRE(barD_count == 0);
        REQUIRE(baz_count == 1);
        REQUIRE(baz0_count == 1);
        REQUIRE(baz1_count == 1);
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
        REQUIRE(a_cursor->baz[0] == 1);
        REQUIRE(a_count == 1);
        REQUIRE(b_count == 0);
        REQUIRE(foo_count == 0);
        REQUIRE(bar_count == 0);
        REQUIRE(barD_count == 0);
        REQUIRE(baz_count == 1);
        REQUIRE(baz0_count == 1);
        REQUIRE(baz1_count == 0);
    }
}

TEST_CASE(value)
{
    using namespace shared_state_suite_ns;

    bool a_modified = false;

    hi::observer<int> a;
    auto a_cbt = a.subscribe([&a_modified](auto...) {
        a_modified = true;
    });
    REQUIRE(not a_modified);
    REQUIRE(*a == 0);
    a_modified = false;

    a = 1;
    REQUIRE(a_modified);
    REQUIRE(*a == 1);
    a_modified = false;
}

TEST_CASE(chain1)
{
    using namespace shared_state_suite_ns;

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

    REQUIRE(not a_modified);
    REQUIRE(not b_modified);
    REQUIRE(*a == 0);
    REQUIRE(*b == 0);
    a_modified = false;
    b_modified = false;

    a = 1;
    b = 2;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(*a == 1);
    REQUIRE(*b == 2);
    a_modified = false;
    b_modified = false;

    a = b;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(*a == 2);
    REQUIRE(*b == 2);
    a_modified = false;

    b = 3;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(*a == 3);
    REQUIRE(*b == 3);
    a_modified = false;
    b_modified = false;

    a = 4;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(*a == 4);
    REQUIRE(*b == 4);
    a_modified = false;
    b_modified = false;
}

TEST_CASE(chain2)
{
    using namespace shared_state_suite_ns;

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

    REQUIRE(not a_modified);
    REQUIRE(not b_modified);
    REQUIRE(not c_modified);
    REQUIRE(*a == 0);
    REQUIRE(*b == 0);
    REQUIRE(*c == 0);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 1;
    b = 2;
    c = 3;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 1);
    REQUIRE(*b == 2);
    REQUIRE(*c == 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = b;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(not c_modified);
    REQUIRE(*a == 2);
    REQUIRE(*b == 2);
    REQUIRE(*c == 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = c;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 3);
    REQUIRE(*b == 3);
    REQUIRE(*c == 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    c = 4;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 4);
    REQUIRE(*b == 4);
    REQUIRE(*c == 4);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = 5;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 5);
    REQUIRE(*b == 5);
    REQUIRE(*c == 5);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 6;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 6);
    REQUIRE(*b == 6);
    REQUIRE(*c == 6);
    a_modified = false;
    b_modified = false;
    c_modified = false;
}

TEST_CASE(chain3)
{
    using namespace shared_state_suite_ns;

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

    REQUIRE(not a_modified);
    REQUIRE(not b_modified);
    REQUIRE(not c_modified);
    REQUIRE(*a == 0);
    REQUIRE(*b == 0);
    REQUIRE(*c == 0);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 1;
    b = 2;
    c = 3;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 1);
    REQUIRE(*b == 2);
    REQUIRE(*c == 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = c;
    REQUIRE(not a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 1);
    REQUIRE(*b == 3);
    REQUIRE(*c == 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = b;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 3);
    REQUIRE(*b == 3);
    REQUIRE(*c == 3);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    c = 4;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 4);
    REQUIRE(*b == 4);
    REQUIRE(*c == 4);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    b = 5;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 5);
    REQUIRE(*b == 5);
    REQUIRE(*c == 5);
    a_modified = false;
    b_modified = false;
    c_modified = false;

    a = 6;
    REQUIRE(a_modified);
    REQUIRE(b_modified);
    REQUIRE(c_modified);
    REQUIRE(*a == 6);
    REQUIRE(*b == 6);
    REQUIRE(*c == 6);
    a_modified = false;
    b_modified = false;
    c_modified = false;
}

static void callback1(int new_value)
{
    REQUIRE(new_value == 42);
}

static void callback2(int const& new_value)
{
    REQUIRE(new_value == 42);
}

TEST_CASE(callback)
{
    using namespace shared_state_suite_ns;

    auto a = hi::observer<int>{1};

    // This tests if we can both subscribe a callback that accepts the
    // argument by value or by const reference.
    auto cbt1 = a.subscribe(callback1);
    auto cbt2 = a.subscribe(callback2);

    a = 42;
}

TEST_CASE(convenience_operators)
{
    auto a = hi::observer<int>{};
    REQUIRE((a == 0));

    a = 1;
    REQUIRE((a == 1));

    a += 2;
    REQUIRE((a == 3));
}

}; // TEST_SUITE(shared_state_suite)
