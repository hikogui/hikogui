// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/int_carry.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>
#include <list>

using namespace std;
using namespace tt;

template<typename T>
class int_carry_test : public ::testing::Test {
public:
    typedef std::list<T> List;
    static T shared_;
    T value_;
};

using int_carry_test_types =
    ::testing::Types<unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;
TYPED_TEST_SUITE(int_carry_test, int_carry_test_types);

TYPED_TEST(int_carry_test, Add)
{
    std::pair<TypeParam,TypeParam> r;

    TypeParam zero = 0;
    TypeParam one = 1;
    TypeParam two = 2;
    TypeParam three = 3;
    TypeParam maximum = numeric_limits<TypeParam>::max();
    TypeParam high = maximum - 1;

    r = add_carry(zero, zero, zero);
    ASSERT_EQ(r.first, zero);
    ASSERT_EQ(r.second, zero);

    r = add_carry(zero, zero, one);
    ASSERT_EQ(r.first, one);
    ASSERT_EQ(r.second, zero);

    r = add_carry(zero, one, zero);
    ASSERT_EQ(r.first, one);
    ASSERT_EQ(r.second, zero);

    r = add_carry(zero, one, one);
    ASSERT_EQ(r.first, two);
    ASSERT_EQ(r.second, zero);

        r = add_carry(one, zero, zero);
    ASSERT_EQ(r.first, one);
    ASSERT_EQ(r.second, zero);

    r = add_carry(one, zero, one);
    ASSERT_EQ(r.first, two);
    ASSERT_EQ(r.second, zero);

    r = add_carry(one, one, zero);
    ASSERT_EQ(r.first, two);
    ASSERT_EQ(r.second, zero);

    r = add_carry(one, one, one);
    ASSERT_EQ(r.first, three);
    ASSERT_EQ(r.second, zero);

    r = add_carry(high, zero, zero);
    ASSERT_EQ(r.first, high);
    ASSERT_EQ(r.second, zero);

    r = add_carry(high, zero, one);
    ASSERT_EQ(r.first, maximum);
    ASSERT_EQ(r.second, zero);

    r = add_carry(high, one, zero);
    ASSERT_EQ(r.first, maximum);
    ASSERT_EQ(r.second, zero);

    r = add_carry(high, one, one);
    ASSERT_EQ(r.first, zero);
    ASSERT_EQ(r.second, one);

    r = add_carry(zero, high, zero);
    ASSERT_EQ(r.first, high);
    ASSERT_EQ(r.second, zero);

    r = add_carry(zero, high, one);
    ASSERT_EQ(r.first, maximum);
    ASSERT_EQ(r.second, zero);

    r = add_carry(one, high, zero);
    ASSERT_EQ(r.first, maximum);
    ASSERT_EQ(r.second, zero);

    r = add_carry(one, high, one);
    ASSERT_EQ(r.first, zero);
    ASSERT_EQ(r.second, one);

    r = add_carry(maximum, zero, zero);
    ASSERT_EQ(r.first, maximum);
    ASSERT_EQ(r.second, zero);

    r = add_carry(maximum, zero, one);
    ASSERT_EQ(r.first, zero);
    ASSERT_EQ(r.second, one);

    r = add_carry(maximum, one, zero);
    ASSERT_EQ(r.first, zero);
    ASSERT_EQ(r.second, one);

    r = add_carry(maximum, one, one);
    ASSERT_EQ(r.first, one);
    ASSERT_EQ(r.second, one);

    r = add_carry(zero, maximum, zero);
    ASSERT_EQ(r.first, maximum);
    ASSERT_EQ(r.second, zero);

    r = add_carry(zero, maximum, one);
    ASSERT_EQ(r.first, zero);
    ASSERT_EQ(r.second, one);

    r = add_carry(one, maximum, zero);
    ASSERT_EQ(r.first, zero);
    ASSERT_EQ(r.second, one);

    r = add_carry(one, maximum, one);
    ASSERT_EQ(r.first, one);
    ASSERT_EQ(r.second, one);
}

