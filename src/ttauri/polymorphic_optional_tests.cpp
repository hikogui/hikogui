// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/polymorphic_optional.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

struct A {
    int hello = 10;

    virtual ~A() {}

    virtual int foo() const noexcept {
        return 1;
    }
};

struct B: public A {
    int foo() const noexcept override {
        return 2;
    }
};

struct C: public A {
    int world = 20;

    int foo() const noexcept override {
        return 3;
    }
};

TEST(polymorphic_optional, assignment) {
    std::array<polymorphic_optional<A,sizeof(C)>, 3> values;

    static_assert(is_decayed_derived_from_v<B,A>);

    values[0] = A{};
    values[1] = B{};
    values[2] = C{};
    ASSERT_EQ(values[0]->foo(), 1);
    ASSERT_EQ(values[1]->foo(), 2);
    ASSERT_EQ(values[2]->foo(), 3);
}

