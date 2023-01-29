// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "cast.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

namespace cast_tests {

struct A {
    virtual ~A() = default;
    virtual int foo() const
    {
        return 42;
    }
};

struct B : A {
    int foo() const override
    {
        return 5;
    }
};

} // namespace cast_tests

TEST(cast, up_cast_ref)
{
    cast_tests::B t = cast_tests::B{};

    cast_tests::A& a1 = hi::up_cast<cast_tests::A&>(t);
    cast_tests::A const& a2 = hi::up_cast<cast_tests::A const&>(t);
    cast_tests::B& b1 = hi::up_cast<cast_tests::B&>(t);
    cast_tests::B const& b2 = hi::up_cast<cast_tests::B const&>(t);
    ASSERT_EQ(t.foo(), 5);
    ASSERT_EQ(a1.foo(), 5);
    ASSERT_EQ(a2.foo(), 5);
    ASSERT_EQ(b1.foo(), 5);
    ASSERT_EQ(b2.foo(), 5);
}

TEST(cast, up_cast_const_ref)
{
    cast_tests::B const t = cast_tests::B{};

    cast_tests::A const& a = hi::up_cast<cast_tests::A const&>(t);
    cast_tests::B const& b = hi::up_cast<cast_tests::B const&>(t);
    ASSERT_EQ(t.foo(), 5);
    ASSERT_EQ(a.foo(), 5);
    ASSERT_EQ(b.foo(), 5);
}

TEST(cast, up_cast_ptr)
{
    cast_tests::B t = cast_tests::B{};

    cast_tests::A const *const a1 = hi::up_cast<cast_tests::A *>(&t);
    cast_tests::A const *const a2 = hi::up_cast<cast_tests::A const *>(&t);
    cast_tests::B const *const b1 = hi::up_cast<cast_tests::B *>(&t);
    cast_tests::B const *const b2 = hi::up_cast<cast_tests::B const *>(&t);
    ASSERT_EQ(t.foo(), 5);
    ASSERT_EQ(a1->foo(), 5);
    ASSERT_EQ(a2->foo(), 5);
    ASSERT_EQ(b1->foo(), 5);
    ASSERT_EQ(b2->foo(), 5);
}

TEST(cast, up_cast_const_ptr)
{
    cast_tests::B const t = cast_tests::B{};

    cast_tests::A const *a = hi::up_cast<cast_tests::A const *>(&t);
    cast_tests::B const *b = hi::up_cast<cast_tests::B const *>(&t);
    ASSERT_EQ(t.foo(), 5);
    ASSERT_EQ(a->foo(), 5);
    ASSERT_EQ(b->foo(), 5);
}

TEST(cast, up_cast_nullptr)
{
    cast_tests::B *t = nullptr;

    cast_tests::A const *const a1 = hi::up_cast<cast_tests::A *>(t);
    cast_tests::A const *const a2 = hi::up_cast<cast_tests::A const *>(t);
    cast_tests::B const *const b1 = hi::up_cast<cast_tests::B *>(t);
    cast_tests::B const *const b2 = hi::up_cast<cast_tests::B const *>(t);
    cast_tests::B const *const n1 = hi::up_cast<cast_tests::B *>(nullptr);
    cast_tests::B const *const n2 = hi::up_cast<cast_tests::B const *>(nullptr);
    ASSERT_EQ(t, nullptr);
    ASSERT_EQ(a1, nullptr);
    ASSERT_EQ(a2, nullptr);
    ASSERT_EQ(b1, nullptr);
    ASSERT_EQ(b2, nullptr);
    ASSERT_EQ(n1, nullptr);
    ASSERT_EQ(n2, nullptr);
}

TEST(cast, up_cast_const_nullptr)
{
    cast_tests::B const *t = nullptr;

    cast_tests::A const *a = hi::up_cast<cast_tests::A const *>(t);
    cast_tests::B const *b = hi::up_cast<cast_tests::B const *>(t);
    ASSERT_EQ(t, nullptr);
    ASSERT_EQ(a, nullptr);
    ASSERT_EQ(b, nullptr);
}

TEST(cast, down_cast_ref)
{
    cast_tests::B tmp = cast_tests::B{};
    cast_tests::A& t = tmp;

    cast_tests::A& a1 = hi::down_cast<cast_tests::A&>(t);
    cast_tests::A const& a2 = hi::down_cast<cast_tests::A const&>(t);
    cast_tests::B& b1 = hi::down_cast<cast_tests::B&>(t);
    cast_tests::B const& b2 = hi::down_cast<cast_tests::B const&>(t);
    ASSERT_EQ(t.foo(), 5);
    ASSERT_EQ(a1.foo(), 5);
    ASSERT_EQ(a2.foo(), 5);
    ASSERT_EQ(b1.foo(), 5);
    ASSERT_EQ(b2.foo(), 5);
}

TEST(cast, down_cast_const_ref)
{
    cast_tests::B const tmp = cast_tests::B{};
    cast_tests::A const& t = tmp;

    cast_tests::A const& a = hi::down_cast<cast_tests::A const&>(t);
    cast_tests::B const& b = hi::down_cast<cast_tests::B const&>(t);
    ASSERT_EQ(t.foo(), 5);
    ASSERT_EQ(a.foo(), 5);
    ASSERT_EQ(b.foo(), 5);
}

TEST(cast, down_cast_ptr)
{
    cast_tests::B tmp = cast_tests::B{};
    cast_tests::A& t = tmp;

    cast_tests::A const *const a1 = hi::down_cast<cast_tests::A *>(&t);
    cast_tests::A const *const a2 = hi::down_cast<cast_tests::A const *>(&t);
    cast_tests::B const *const b1 = hi::down_cast<cast_tests::B *>(&t);
    cast_tests::B const *const b2 = hi::down_cast<cast_tests::B const *>(&t);
    ASSERT_EQ(t.foo(), 5);
    ASSERT_EQ(a1->foo(), 5);
    ASSERT_EQ(a2->foo(), 5);
    ASSERT_EQ(b1->foo(), 5);
    ASSERT_EQ(b2->foo(), 5);
}

TEST(cast, down_cast_const_ptr)
{
    cast_tests::B const tmp = cast_tests::B{};
    cast_tests::A const& t = tmp;

    cast_tests::A const *a = hi::down_cast<cast_tests::A const *>(&t);
    cast_tests::B const *b = hi::down_cast<cast_tests::B const *>(&t);
    ASSERT_EQ(t.foo(), 5);
    ASSERT_EQ(a->foo(), 5);
    ASSERT_EQ(b->foo(), 5);
}

TEST(cast, down_cast_nullptr)
{
    cast_tests::A *t = nullptr;

    cast_tests::A const *const a1 = hi::down_cast<cast_tests::A *>(t);
    cast_tests::A const *const a2 = hi::down_cast<cast_tests::A const *>(t);
    cast_tests::B const *const b1 = hi::down_cast<cast_tests::B *>(t);
    cast_tests::B const *const b2 = hi::down_cast<cast_tests::B const *>(t);
    cast_tests::B const *const n1 = hi::down_cast<cast_tests::B *>(nullptr);
    cast_tests::B const *const n2 = hi::down_cast<cast_tests::B const *>(nullptr);
    ASSERT_EQ(t, nullptr);
    ASSERT_EQ(a1, nullptr);
    ASSERT_EQ(a2, nullptr);
    ASSERT_EQ(b1, nullptr);
    ASSERT_EQ(b2, nullptr);
    ASSERT_EQ(n1, nullptr);
    ASSERT_EQ(n2, nullptr);
}

TEST(cast, down_cast_cont_nullptr)
{
    cast_tests::A const *t = nullptr;

    cast_tests::A const *a = hi::down_cast<cast_tests::A const *>(t);
    cast_tests::B const *b = hi::down_cast<cast_tests::B const *>(t);
    ASSERT_EQ(t, nullptr);
    ASSERT_EQ(a, nullptr);
    ASSERT_EQ(b, nullptr);
}
