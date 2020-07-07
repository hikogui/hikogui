// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/polymorphic_value.hpp"
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

TEST(PolymorphicValue, Assignment) {
    std::array<polymorphic_value<A,sizeof(C)>, 3> values;

    values[0] = A{};
    values[1] = B{};
    values[2] = C{};
    ASSERT_EQ(values[0]->foo(), 1);
    ASSERT_EQ(values[1]->foo(), 2);
    ASSERT_EQ(values[2]->foo(), 3);
}

