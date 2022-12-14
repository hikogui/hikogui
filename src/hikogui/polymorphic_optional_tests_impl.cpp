// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/polymorphic_optional.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <type_traits>

namespace polymorphic_optional_tests {
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
}
TEST(polymorphic_optional, assignment)
{
    std::array<hi::polymorphic_optional<polymorphic_optional_tests::A, sizeof(polymorphic_optional_tests::C)>, 3> values;

    static_assert(std::is_base_of_v<polymorphic_optional_tests::A, polymorphic_optional_tests::B>);

    values[0] = polymorphic_optional_tests::A{};
    values[1] = polymorphic_optional_tests::B{};
    values[2] = polymorphic_optional_tests::C{};
    ASSERT_EQ(values[0]->foo(), 1);
    ASSERT_EQ(values[1]->foo(), 2);
    ASSERT_EQ(values[2]->foo(), 3);
}
