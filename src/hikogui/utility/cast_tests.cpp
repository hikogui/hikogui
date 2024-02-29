// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "cast.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(case_suite) {


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

TEST_CASE(up_cast_ref)
{
    B t = B{};

    A& a1 = hi::up_cast<A&>(t);
    A const& a2 = hi::up_cast<A const&>(t);
    B& b1 = hi::up_cast<B&>(t);
    B const& b2 = hi::up_cast<B const&>(t);
    REQUIRE(t.foo() == 5);
    REQUIRE(a1.foo() == 5);
    REQUIRE(a2.foo() == 5);
    REQUIRE(b1.foo() == 5);
    REQUIRE(b2.foo() == 5);
}

TEST_CASE(up_cast_const_ref)
{
    B const t = B{};

    A const& a = hi::up_cast<A const&>(t);
    B const& b = hi::up_cast<B const&>(t);
    REQUIRE(t.foo() == 5);
    REQUIRE(a.foo() == 5);
    REQUIRE(b.foo() == 5);
}

TEST_CASE(up_cast_ptr)
{
    B t = B{};

    A const *const a1 = hi::up_cast<A *>(&t);
    A const *const a2 = hi::up_cast<A const *>(&t);
    B const *const b1 = hi::up_cast<B *>(&t);
    B const *const b2 = hi::up_cast<B const *>(&t);
    REQUIRE(t.foo() == 5);
    REQUIRE(a1->foo() == 5);
    REQUIRE(a2->foo() == 5);
    REQUIRE(b1->foo() == 5);
    REQUIRE(b2->foo() == 5);
}

TEST_CASE(up_cast_const_ptr)
{
    B const t = B{};

    A const *a = hi::up_cast<A const *>(&t);
    B const *b = hi::up_cast<B const *>(&t);
    REQUIRE(t.foo() == 5);
    REQUIRE(a->foo() == 5);
    REQUIRE(b->foo() == 5);
}

TEST_CASE(up_cast_nullptr)
{
    B *t = nullptr;

    A const *const a1 = hi::up_cast<A *>(t);
    A const *const a2 = hi::up_cast<A const *>(t);
    B const *const b1 = hi::up_cast<B *>(t);
    B const *const b2 = hi::up_cast<B const *>(t);
    B const *const n1 = hi::up_cast<B *>(nullptr);
    B const *const n2 = hi::up_cast<B const *>(nullptr);
    REQUIRE(t == nullptr);
    REQUIRE(a1 == nullptr);
    REQUIRE(a2 == nullptr);
    REQUIRE(b1 == nullptr);
    REQUIRE(b2 == nullptr);
    REQUIRE(n1 == nullptr);
    REQUIRE(n2 == nullptr);
}

TEST_CASE(up_cast_const_nullptr)
{
    B const *t = nullptr;

    A const *a = hi::up_cast<A const *>(t);
    B const *b = hi::up_cast<B const *>(t);
    REQUIRE(t == nullptr);
    REQUIRE(a == nullptr);
    REQUIRE(b == nullptr);
}

TEST_CASE(down_cast_ref)
{
    B tmp = B{};
    A& t = tmp;

    A& a1 = hi::down_cast<A&>(t);
    A const& a2 = hi::down_cast<A const&>(t);
    B& b1 = hi::down_cast<B&>(t);
    B const& b2 = hi::down_cast<B const&>(t);
    REQUIRE(t.foo() == 5);
    REQUIRE(a1.foo() == 5);
    REQUIRE(a2.foo() == 5);
    REQUIRE(b1.foo() == 5);
    REQUIRE(b2.foo() == 5);
}

TEST_CASE(down_cast_const_ref)
{
    B const tmp = B{};
    A const& t = tmp;

    A const& a = hi::down_cast<A const&>(t);
    B const& b = hi::down_cast<B const&>(t);
    REQUIRE(t.foo() == 5);
    REQUIRE(a.foo() == 5);
    REQUIRE(b.foo() == 5);
}

TEST_CASE(down_cast_ptr)
{
    B tmp = B{};
    A& t = tmp;

    A const *const a1 = hi::down_cast<A *>(&t);
    A const *const a2 = hi::down_cast<A const *>(&t);
    B const *const b1 = hi::down_cast<B *>(&t);
    B const *const b2 = hi::down_cast<B const *>(&t);
    REQUIRE(t.foo() == 5);
    REQUIRE(a1->foo() == 5);
    REQUIRE(a2->foo() == 5);
    REQUIRE(b1->foo() == 5);
    REQUIRE(b2->foo() == 5);
}

TEST_CASE(down_cast_const_ptr)
{
    B const tmp = B{};
    A const& t = tmp;

    A const *a = hi::down_cast<A const *>(&t);
    B const *b = hi::down_cast<B const *>(&t);
    REQUIRE(t.foo() == 5);
    REQUIRE(a->foo() == 5);
    REQUIRE(b->foo() == 5);
}

TEST_CASE(down_cast_nullptr)
{
    A *t = nullptr;

    A const *const a1 = hi::down_cast<A *>(t);
    A const *const a2 = hi::down_cast<A const *>(t);
    B const *const b1 = hi::down_cast<B *>(t);
    B const *const b2 = hi::down_cast<B const *>(t);
    B const *const n1 = hi::down_cast<B *>(nullptr);
    B const *const n2 = hi::down_cast<B const *>(nullptr);
    REQUIRE(t == nullptr);
    REQUIRE(a1 == nullptr);
    REQUIRE(a2 == nullptr);
    REQUIRE(b1 == nullptr);
    REQUIRE(b2 == nullptr);
    REQUIRE(n1 == nullptr);
    REQUIRE(n2 == nullptr);
}

TEST_CASE(down_cast_cont_nullptr)
{
    A const *t = nullptr;

    A const *a = hi::down_cast<A const *>(t);
    B const *b = hi::down_cast<B const *>(t);
    REQUIRE(t == nullptr);
    REQUIRE(a == nullptr);
    REQUIRE(b == nullptr);
}

};
