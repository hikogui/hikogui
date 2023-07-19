// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "int_carry.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>
#include <list>

using namespace std;
using namespace hi;

template<typename T>
class int_carry_test : public ::testing::Test {
public:
    typedef std::list<T> List;
    static T shared_;
    T value_;
};

using int_carry_test_types = ::testing::Types<unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;
TYPED_TEST_SUITE(int_carry_test, int_carry_test_types);

TYPED_TEST(int_carry_test, Add)
{
    constexpr TypeParam zero = 0;
    constexpr TypeParam one = 1;
    constexpr TypeParam two = 2;
    constexpr TypeParam three = 3;
    constexpr TypeParam maximum = numeric_limits<TypeParam>::max();
    constexpr TypeParam high = maximum - 1;

    ASSERT_EQ(add_carry(zero, zero, zero), std::pair(zero, zero));
    ASSERT_EQ(add_carry(zero, zero, one), std::pair(one, zero));
    ASSERT_EQ(add_carry(zero, one, zero), std::pair(one, zero));
    ASSERT_EQ(add_carry(zero, one, one), std::pair(two, zero));
    ASSERT_EQ(add_carry(one, zero, zero), std::pair(one, zero));
    ASSERT_EQ(add_carry(one, zero, one), std::pair(two, zero));
    ASSERT_EQ(add_carry(one, one, zero), std::pair(two, zero));
    ASSERT_EQ(add_carry(one, one, one), std::pair(three, zero));
    ASSERT_EQ(add_carry(high, zero, zero), std::pair(high, zero));
    ASSERT_EQ(add_carry(high, zero, one), std::pair(maximum, zero));
    ASSERT_EQ(add_carry(high, one, zero), std::pair(maximum, zero));
    ASSERT_EQ(add_carry(high, one, one), std::pair(zero, one));
    ASSERT_EQ(add_carry(zero, high, zero), std::pair(high, zero));
    ASSERT_EQ(add_carry(zero, high, one), std::pair(maximum, zero));
    ASSERT_EQ(add_carry(one, high, zero), std::pair(maximum, zero));
    ASSERT_EQ(add_carry(one, high, one), std::pair(zero, one));
    ASSERT_EQ(add_carry(maximum, zero, zero), std::pair(maximum, zero));
    ASSERT_EQ(add_carry(maximum, zero, one), std::pair(zero, one));
    ASSERT_EQ(add_carry(maximum, one, zero), std::pair(zero, one));
    ASSERT_EQ(add_carry(maximum, one, one), std::pair(one, one));
    ASSERT_EQ(add_carry(zero, maximum, zero), std::pair(maximum, zero));
    ASSERT_EQ(add_carry(zero, maximum, one), std::pair(zero, one));
    ASSERT_EQ(add_carry(one, maximum, zero), std::pair(zero, one));
    ASSERT_EQ(add_carry(one, maximum, one), std::pair(one, one));

    static_assert(add_carry(zero, zero, zero) == std::pair(zero, zero));
    static_assert(add_carry(zero, zero, one) == std::pair(one, zero));
    static_assert(add_carry(zero, one, zero) == std::pair(one, zero));
    static_assert(add_carry(zero, one, one) == std::pair(two, zero));
    static_assert(add_carry(one, zero, zero) == std::pair(one, zero));
    static_assert(add_carry(one, zero, one) == std::pair(two, zero));
    static_assert(add_carry(one, one, zero) == std::pair(two, zero));
    static_assert(add_carry(one, one, one) == std::pair(three, zero));
    static_assert(add_carry(high, zero, zero) == std::pair(high, zero));
    static_assert(add_carry(high, zero, one) == std::pair(maximum, zero));
    static_assert(add_carry(high, one, zero) == std::pair(maximum, zero));
    static_assert(add_carry(high, one, one) == std::pair(zero, one));
    static_assert(add_carry(zero, high, zero) == std::pair(high, zero));
    static_assert(add_carry(zero, high, one) == std::pair(maximum, zero));
    static_assert(add_carry(one, high, zero) == std::pair(maximum, zero));
    static_assert(add_carry(one, high, one) == std::pair(zero, one));
    static_assert(add_carry(maximum, zero, zero) == std::pair(maximum, zero));
    static_assert(add_carry(maximum, zero, one) == std::pair(zero, one));
    static_assert(add_carry(maximum, one, zero) == std::pair(zero, one));
    static_assert(add_carry(maximum, one, one) == std::pair(one, one));
    static_assert(add_carry(zero, maximum, zero) == std::pair(maximum, zero));
    static_assert(add_carry(zero, maximum, one) == std::pair(zero, one));
    static_assert(add_carry(one, maximum, zero) == std::pair(zero, one));
    static_assert(add_carry(one, maximum, one) == std::pair(one, one));
}
