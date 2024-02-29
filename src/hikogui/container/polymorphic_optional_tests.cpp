// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "polymorphic_optional.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(polymorphic_optional) {

struct A {
    int hello = 10;

    virtual ~A() {}

    virtual int foo() const noexcept
    {
        return 1;
    }
};

struct B : public A {
    int foo() const noexcept override
    {
        return 2;
    }
};

struct C : public A {
    int world = 20;

    int foo() const noexcept override
    {
        return 3;
    }
};

TEST_CASE(assignment)
{
    std::array<hi::polymorphic_optional<A, sizeof(C)>, 3> values;

    static_assert(std::is_base_of_v<A, B>);

    values[0] = A{};
    values[1] = B{};
    values[2] = C{};
    REQUIRE(values[0]->foo() == 1);
    REQUIRE(values[1]->foo() == 2);
    REQUIRE(values[2]->foo() == 3);
}

};
