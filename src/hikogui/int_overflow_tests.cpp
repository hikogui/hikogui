// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/int_overflow.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>
#include <list>

using namespace std;
using namespace hi;

template<typename T>
class SignedIntOverflow : public ::testing::Test {
public:
    typedef std::list<T> List;
    static T shared_;
    T value_;
};

template<typename T>
class UnsignedIntOverflow : public ::testing::Test {
public:
    typedef std::list<T> List;
    static T shared_;
    T value_;
};

using SignedIntOverflow_Types = ::testing::Types<signed char, signed short, signed int, signed long, signed long long>;
TYPED_TEST_SUITE(SignedIntOverflow, SignedIntOverflow_Types);

using UnsignedIntOverflow_Types =
    ::testing::Types<unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;
TYPED_TEST_SUITE(UnsignedIntOverflow, UnsignedIntOverflow_Types);

TYPED_TEST(SignedIntOverflow, Add)
{
    TypeParam r;

    TypeParam zero = 0;
    TypeParam one = 1;
    TypeParam minTwo = -2;
    TypeParam minOne = -1;
    TypeParam two = 2;
    TypeParam maximum = numeric_limits<TypeParam>::max();
    TypeParam high = maximum - 1;
    TypeParam lessHigh = maximum - 2;
    TypeParam minimum = numeric_limits<TypeParam>::min();
    TypeParam low = minimum + 1;
    TypeParam lessLow = minimum + 2;

    ASSERT_TRUE(add_overflow(minimum, minimum, &r));
    ASSERT_TRUE(add_overflow(minimum, low, &r));
    ASSERT_TRUE(add_overflow(minimum, minOne, &r));
    ASSERT_FALSE(add_overflow(minimum, zero, &r));
    ASSERT_EQ(r, minimum);
    ASSERT_FALSE(add_overflow(minimum, one, &r));
    ASSERT_EQ(r, low);
    ASSERT_FALSE(add_overflow(minimum, high, &r));
    ASSERT_EQ(r, minTwo);
    ASSERT_FALSE(add_overflow(minimum, maximum, &r));
    ASSERT_EQ(r, minOne);

    ASSERT_TRUE(add_overflow(low, minimum, &r));
    ASSERT_TRUE(add_overflow(low, low, &r));
    ASSERT_FALSE(add_overflow(low, minOne, &r));
    ASSERT_EQ(r, minimum);
    ASSERT_FALSE(add_overflow(low, zero, &r));
    ASSERT_EQ(r, low);
    ASSERT_FALSE(add_overflow(low, one, &r));
    ASSERT_EQ(r, lessLow);
    ASSERT_FALSE(add_overflow(low, high, &r));
    ASSERT_EQ(r, minOne);
    ASSERT_FALSE(add_overflow(low, maximum, &r));
    ASSERT_EQ(r, zero);

    ASSERT_TRUE(add_overflow(minOne, minimum, &r));
    ASSERT_FALSE(add_overflow(minOne, low, &r));
    ASSERT_EQ(r, minimum);
    ASSERT_FALSE(add_overflow(minOne, minOne, &r));
    ASSERT_EQ(r, minTwo);
    ASSERT_FALSE(add_overflow(minOne, zero, &r));
    ASSERT_EQ(r, minOne);
    ASSERT_FALSE(add_overflow(minOne, one, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(add_overflow(minOne, high, &r));
    ASSERT_EQ(r, lessHigh);
    ASSERT_FALSE(add_overflow(minOne, maximum, &r));
    ASSERT_EQ(r, high);

    ASSERT_FALSE(add_overflow(zero, minimum, &r));
    ASSERT_EQ(r, minimum);
    ASSERT_FALSE(add_overflow(zero, low, &r));
    ASSERT_EQ(r, low);
    ASSERT_FALSE(add_overflow(zero, minOne, &r));
    ASSERT_EQ(r, minOne);
    ASSERT_FALSE(add_overflow(zero, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(add_overflow(zero, one, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(add_overflow(zero, high, &r));
    ASSERT_EQ(r, high);
    ASSERT_FALSE(add_overflow(zero, maximum, &r));
    ASSERT_EQ(r, maximum);

    ASSERT_FALSE(add_overflow(one, minimum, &r));
    ASSERT_EQ(r, low);
    ASSERT_FALSE(add_overflow(one, low, &r));
    ASSERT_EQ(r, lessLow);
    ASSERT_FALSE(add_overflow(one, minOne, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(add_overflow(one, zero, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(add_overflow(one, one, &r));
    ASSERT_EQ(r, two);
    ASSERT_FALSE(add_overflow(one, high, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(one, maximum, &r));

    ASSERT_FALSE(add_overflow(high, minimum, &r));
    ASSERT_EQ(r, minTwo);
    ASSERT_FALSE(add_overflow(high, low, &r));
    ASSERT_EQ(r, minOne);
    ASSERT_FALSE(add_overflow(high, minOne, &r));
    ASSERT_EQ(r, lessHigh);
    ASSERT_FALSE(add_overflow(high, zero, &r));
    ASSERT_EQ(r, high);
    ASSERT_FALSE(add_overflow(high, one, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(high, high, &r));
    ASSERT_TRUE(add_overflow(high, maximum, &r));

    ASSERT_FALSE(add_overflow(maximum, minimum, &r));
    ASSERT_EQ(r, minOne);
    ASSERT_FALSE(add_overflow(maximum, low, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(add_overflow(maximum, minOne, &r));
    ASSERT_EQ(r, high);
    ASSERT_FALSE(add_overflow(maximum, zero, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(maximum, one, &r));
    ASSERT_TRUE(add_overflow(maximum, high, &r));
    ASSERT_TRUE(add_overflow(maximum, maximum, &r));
}

TYPED_TEST(UnsignedIntOverflow, Add)
{
    TypeParam r;

    TypeParam zero = 0;
    TypeParam one = 1;
    TypeParam two = 2;
    TypeParam maximum = numeric_limits<TypeParam>::max();
    TypeParam high = maximum - 1;

    ASSERT_FALSE(add_overflow(zero, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(add_overflow(zero, one, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(add_overflow(zero, high, &r));
    ASSERT_EQ(r, high);
    ASSERT_FALSE(add_overflow(zero, maximum, &r));
    ASSERT_EQ(r, maximum);

    ASSERT_FALSE(add_overflow(one, zero, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(add_overflow(one, one, &r));
    ASSERT_EQ(r, two);
    ASSERT_FALSE(add_overflow(one, high, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(one, maximum, &r));

    ASSERT_FALSE(add_overflow(high, zero, &r));
    ASSERT_EQ(r, high);
    ASSERT_FALSE(add_overflow(high, one, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(high, high, &r));
    ASSERT_TRUE(add_overflow(high, maximum, &r));

    ASSERT_FALSE(add_overflow(maximum, zero, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(maximum, one, &r));
    ASSERT_TRUE(add_overflow(maximum, high, &r));
    ASSERT_TRUE(add_overflow(maximum, maximum, &r));
}

TYPED_TEST(SignedIntOverflow, Subtract)
{
    TypeParam r;

    TypeParam zero = 0;
    TypeParam one = 1;
    TypeParam minTwo = -2;
    TypeParam minOne = -1;
    TypeParam two = 2;
    TypeParam maximum = numeric_limits<TypeParam>::max();
    TypeParam high = maximum - 1;
    TypeParam lessHigh = maximum - 2;
    TypeParam minimum = numeric_limits<TypeParam>::min();
    TypeParam low = minimum + 1;
    TypeParam lessLow = minimum + 2;
    TypeParam lessLessLow = minimum + 3;

    ASSERT_FALSE(sub_overflow(minimum, minimum, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(sub_overflow(minimum, low, &r));
    ASSERT_EQ(r, minOne);
    ASSERT_FALSE(sub_overflow(minimum, minOne, &r));
    ASSERT_EQ(r, low);
    ASSERT_FALSE(sub_overflow(minimum, zero, &r));
    ASSERT_EQ(r, minimum);
    ASSERT_TRUE(sub_overflow(minimum, one, &r));
    ASSERT_TRUE(sub_overflow(minimum, high, &r));
    ASSERT_TRUE(sub_overflow(minimum, maximum, &r));

    ASSERT_FALSE(sub_overflow(low, minimum, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(sub_overflow(low, low, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(sub_overflow(low, minOne, &r));
    ASSERT_EQ(r, lessLow);
    ASSERT_FALSE(sub_overflow(low, zero, &r));
    ASSERT_EQ(r, low);
    ASSERT_FALSE(sub_overflow(low, one, &r));
    ASSERT_EQ(r, minimum);
    ASSERT_TRUE(sub_overflow(low, high, &r));
    ASSERT_TRUE(sub_overflow(low, maximum, &r));

    ASSERT_FALSE(sub_overflow(minOne, minimum, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_FALSE(sub_overflow(minOne, low, &r));
    ASSERT_EQ(r, high);
    ASSERT_FALSE(sub_overflow(minOne, minOne, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(sub_overflow(minOne, zero, &r));
    ASSERT_EQ(r, minOne);
    ASSERT_FALSE(sub_overflow(minOne, one, &r));
    ASSERT_EQ(r, minTwo);
    ASSERT_FALSE(sub_overflow(minOne, high, &r));
    ASSERT_EQ(r, low);
    ASSERT_FALSE(sub_overflow(minOne, maximum, &r));
    ASSERT_EQ(r, minimum);

    ASSERT_TRUE(sub_overflow(zero, minimum, &r));
    ASSERT_FALSE(sub_overflow(zero, low, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_FALSE(sub_overflow(zero, minOne, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(sub_overflow(zero, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(sub_overflow(zero, one, &r));
    ASSERT_EQ(r, minOne);
    ASSERT_FALSE(sub_overflow(zero, high, &r));
    ASSERT_EQ(r, lessLow);
    ASSERT_FALSE(sub_overflow(zero, maximum, &r));
    ASSERT_EQ(r, low);

    ASSERT_TRUE(sub_overflow(one, minimum, &r));
    ASSERT_TRUE(sub_overflow(one, low, &r));
    ASSERT_FALSE(sub_overflow(one, minOne, &r));
    ASSERT_EQ(r, two);
    ASSERT_FALSE(sub_overflow(one, zero, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(sub_overflow(one, one, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(sub_overflow(one, high, &r));
    ASSERT_EQ(r, lessLessLow);
    ASSERT_FALSE(sub_overflow(one, maximum, &r));
    ASSERT_EQ(r, lessLow);

    ASSERT_TRUE(sub_overflow(high, minimum, &r));
    ASSERT_TRUE(sub_overflow(high, low, &r));
    ASSERT_FALSE(sub_overflow(high, minOne, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_FALSE(sub_overflow(high, zero, &r));
    ASSERT_EQ(r, high);
    ASSERT_FALSE(sub_overflow(high, one, &r));
    ASSERT_EQ(r, lessHigh);
    ASSERT_FALSE(sub_overflow(high, high, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(sub_overflow(high, maximum, &r));
    ASSERT_EQ(r, minOne);

    ASSERT_TRUE(sub_overflow(maximum, minimum, &r));
    ASSERT_TRUE(sub_overflow(maximum, low, &r));
    ASSERT_TRUE(sub_overflow(maximum, minOne, &r));
    ASSERT_FALSE(sub_overflow(maximum, zero, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_FALSE(sub_overflow(maximum, one, &r));
    ASSERT_EQ(r, high);
    ASSERT_FALSE(sub_overflow(maximum, high, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(sub_overflow(maximum, maximum, &r));
    ASSERT_EQ(r, zero);
}

TYPED_TEST(UnsignedIntOverflow, Subtract)
{
    TypeParam r;

    TypeParam zero = 0;
    TypeParam one = 1;
    TypeParam maximum = numeric_limits<TypeParam>::max();
    TypeParam high = maximum - 1;
    TypeParam lessHigh = maximum - 2;

    ASSERT_FALSE(sub_overflow(zero, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_TRUE(sub_overflow(zero, one, &r));
    ASSERT_TRUE(sub_overflow(zero, high, &r));
    ASSERT_TRUE(sub_overflow(zero, maximum, &r));

    ASSERT_FALSE(sub_overflow(one, zero, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(sub_overflow(one, one, &r));
    ASSERT_EQ(r, zero);
    ASSERT_TRUE(sub_overflow(one, high, &r));
    ASSERT_TRUE(sub_overflow(one, maximum, &r));

    ASSERT_FALSE(sub_overflow(high, zero, &r));
    ASSERT_EQ(r, high);
    ASSERT_FALSE(sub_overflow(high, one, &r));
    ASSERT_EQ(r, lessHigh);
    ASSERT_FALSE(sub_overflow(high, high, &r));
    ASSERT_EQ(r, zero);
    ASSERT_TRUE(sub_overflow(high, maximum, &r));

    ASSERT_FALSE(sub_overflow(maximum, zero, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_FALSE(sub_overflow(maximum, one, &r));
    ASSERT_EQ(r, high);
    ASSERT_FALSE(sub_overflow(maximum, high, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(sub_overflow(maximum, maximum, &r));
    ASSERT_EQ(r, zero);
}

TYPED_TEST(SignedIntOverflow, Multiply)
{
    TypeParam r;

    // Arguments
    TypeParam zero = 0;
    TypeParam one = 1;
    TypeParam minOne = -1;
    TypeParam two = 2;
    TypeParam minTwo = -2;
    TypeParam maximum = numeric_limits<TypeParam>::max();
    TypeParam half = maximum / 2;
    TypeParam minimum = numeric_limits<TypeParam>::min();
    TypeParam minHalf = minimum / 2;

    ASSERT_TRUE(mul_overflow(minimum, minimum, &r));
    ASSERT_TRUE(mul_overflow(minimum, minHalf, &r));
    ASSERT_TRUE(mul_overflow(minimum, minTwo, &r));
    ASSERT_TRUE(mul_overflow(minimum, minOne, &r));
    ASSERT_FALSE(mul_overflow(minimum, zero, &r));
    ASSERT_EQ(r, 0);
    ASSERT_FALSE(mul_overflow(minimum, one, &r));
    ASSERT_EQ(r, minimum);
    ASSERT_TRUE(mul_overflow(minimum, two, &r));
    ASSERT_TRUE(mul_overflow(minimum, half, &r));
    ASSERT_TRUE(mul_overflow(minimum, maximum, &r));

    ASSERT_TRUE(mul_overflow(minHalf, minimum, &r));
    ASSERT_TRUE(mul_overflow(minHalf, minHalf, &r));
    ASSERT_TRUE(mul_overflow(minHalf, minTwo, &r));
    ASSERT_FALSE(mul_overflow(minHalf, minOne, &r));
    ASSERT_EQ(r, -minHalf);
    ASSERT_FALSE(mul_overflow(minHalf, zero, &r));
    ASSERT_EQ(r, 0);
    ASSERT_FALSE(mul_overflow(minHalf, one, &r));
    ASSERT_EQ(r, minHalf);
    ASSERT_FALSE(mul_overflow(minHalf, two, &r));
    ASSERT_EQ(r, minimum);
    ASSERT_TRUE(mul_overflow(minHalf, half, &r));
    ASSERT_TRUE(mul_overflow(minHalf, maximum, &r));

    ASSERT_TRUE(mul_overflow(minTwo, minimum, &r));
    ASSERT_TRUE(mul_overflow(minTwo, minHalf, &r));
    ASSERT_FALSE(mul_overflow(minTwo, minTwo, &r));
    ASSERT_EQ(r, 4);
    ASSERT_FALSE(mul_overflow(minTwo, minOne, &r));
    ASSERT_EQ(r, 2);
    ASSERT_FALSE(mul_overflow(minTwo, zero, &r));
    ASSERT_EQ(r, 0);
    ASSERT_FALSE(mul_overflow(minTwo, one, &r));
    ASSERT_EQ(r, -2);
    ASSERT_FALSE(mul_overflow(minTwo, two, &r));
    ASSERT_EQ(r, -4);
    ASSERT_FALSE(mul_overflow(minTwo, half, &r));
    ASSERT_EQ(r, minimum + 2);
    ASSERT_TRUE(mul_overflow(minTwo, maximum, &r));

    ASSERT_TRUE(mul_overflow(minOne, minimum, &r));
    ASSERT_FALSE(mul_overflow(minOne, minHalf, &r));
    ASSERT_EQ(r, -minHalf);
    ASSERT_FALSE(mul_overflow(minOne, minTwo, &r));
    ASSERT_EQ(r, 2);
    ASSERT_FALSE(mul_overflow(minOne, minOne, &r));
    ASSERT_EQ(r, 1);
    ASSERT_FALSE(mul_overflow(minOne, zero, &r));
    ASSERT_EQ(r, 0);
    ASSERT_FALSE(mul_overflow(minOne, one, &r));
    ASSERT_EQ(r, -1);
    ASSERT_FALSE(mul_overflow(minOne, two, &r));
    ASSERT_EQ(r, -2);
    ASSERT_FALSE(mul_overflow(minOne, half, &r));
    ASSERT_EQ(r, -half);
    ASSERT_FALSE(mul_overflow(minOne, maximum, &r));
    ASSERT_EQ(r, -maximum);

    ASSERT_FALSE(mul_overflow(zero, minimum, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, minHalf, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, minTwo, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, minOne, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, one, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, two, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, half, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, maximum, &r));
    ASSERT_EQ(r, zero);

    ASSERT_FALSE(mul_overflow(one, minimum, &r));
    ASSERT_EQ(r, minimum);
    ASSERT_FALSE(mul_overflow(one, minHalf, &r));
    ASSERT_EQ(r, minHalf);
    ASSERT_FALSE(mul_overflow(one, minTwo, &r));
    ASSERT_EQ(r, -2);
    ASSERT_FALSE(mul_overflow(one, minOne, &r));
    ASSERT_EQ(r, -1);
    ASSERT_FALSE(mul_overflow(one, zero, &r));
    ASSERT_EQ(r, 0);
    ASSERT_FALSE(mul_overflow(one, one, &r));
    ASSERT_EQ(r, 1);
    ASSERT_FALSE(mul_overflow(one, two, &r));
    ASSERT_EQ(r, 2);
    ASSERT_FALSE(mul_overflow(one, half, &r));
    ASSERT_EQ(r, half);
    ASSERT_FALSE(mul_overflow(one, maximum, &r));
    ASSERT_EQ(r, maximum);

    ASSERT_TRUE(mul_overflow(two, minimum, &r));
    ASSERT_FALSE(mul_overflow(two, minHalf, &r));
    ASSERT_EQ(r, minimum);
    ASSERT_FALSE(mul_overflow(two, minTwo, &r));
    ASSERT_EQ(r, -4);
    ASSERT_FALSE(mul_overflow(two, minOne, &r));
    ASSERT_EQ(r, -2);
    ASSERT_FALSE(mul_overflow(two, zero, &r));
    ASSERT_EQ(r, 0);
    ASSERT_FALSE(mul_overflow(two, one, &r));
    ASSERT_EQ(r, 2);
    ASSERT_FALSE(mul_overflow(two, two, &r));
    ASSERT_EQ(r, 4);
    ASSERT_FALSE(mul_overflow(two, half, &r));
    ASSERT_EQ(r, maximum - 1);
    ASSERT_TRUE(mul_overflow(two, maximum, &r));

    ASSERT_TRUE(mul_overflow(half, minimum, &r));
    ASSERT_TRUE(mul_overflow(half, minHalf, &r));
    ASSERT_FALSE(mul_overflow(half, minTwo, &r));
    ASSERT_EQ(r, minimum + 2);
    ASSERT_FALSE(mul_overflow(half, minOne, &r));
    ASSERT_EQ(r, -half);
    ASSERT_FALSE(mul_overflow(half, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(half, one, &r));
    ASSERT_EQ(r, half);
    ASSERT_FALSE(mul_overflow(half, two, &r));
    ASSERT_EQ(r, maximum - 1);
    ASSERT_TRUE(mul_overflow(half, half, &r));
    ASSERT_TRUE(mul_overflow(half, maximum, &r));

    ASSERT_TRUE(mul_overflow(maximum, minimum, &r));
    ASSERT_TRUE(mul_overflow(maximum, minHalf, &r));
    ASSERT_TRUE(mul_overflow(maximum, minTwo, &r));
    ASSERT_FALSE(mul_overflow(maximum, minOne, &r));
    ASSERT_EQ(r, minimum + 1);
    ASSERT_FALSE(mul_overflow(maximum, zero, &r));
    ASSERT_EQ(r, 0);
    ASSERT_FALSE(mul_overflow(maximum, one, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_TRUE(mul_overflow(maximum, two, &r));
    ASSERT_TRUE(mul_overflow(maximum, half, &r));
    ASSERT_TRUE(mul_overflow(maximum, maximum, &r));
}

TYPED_TEST(UnsignedIntOverflow, Multiply)
{
    TypeParam r;

    // Arguments
    TypeParam zero = 0;
    TypeParam one = 1;
    TypeParam two = 2;
    TypeParam four = 4;
    TypeParam maximum = numeric_limits<TypeParam>::max();
    TypeParam half = maximum / 2;

    ASSERT_FALSE(mul_overflow(zero, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, one, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, two, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, half, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(zero, maximum, &r));
    ASSERT_EQ(r, zero);

    ASSERT_FALSE(mul_overflow(one, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(one, one, &r));
    ASSERT_EQ(r, one);
    ASSERT_FALSE(mul_overflow(one, two, &r));
    ASSERT_EQ(r, two);
    ASSERT_FALSE(mul_overflow(one, half, &r));
    ASSERT_EQ(r, half);
    ASSERT_FALSE(mul_overflow(one, maximum, &r));
    ASSERT_EQ(r, maximum);

    ASSERT_FALSE(mul_overflow(two, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(two, one, &r));
    ASSERT_EQ(r, two);
    ASSERT_FALSE(mul_overflow(two, two, &r));
    ASSERT_EQ(r, four);
    ASSERT_FALSE(mul_overflow(two, half, &r));
    ASSERT_EQ(r, maximum - 1);
    ASSERT_TRUE(mul_overflow(two, maximum, &r));

    ASSERT_FALSE(mul_overflow(half, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(half, one, &r));
    ASSERT_EQ(r, half);
    ASSERT_FALSE(mul_overflow(half, two, &r));
    ASSERT_EQ(r, maximum - 1);
    ASSERT_TRUE(mul_overflow(half, half, &r));
    ASSERT_TRUE(mul_overflow(half, maximum, &r));

    ASSERT_FALSE(mul_overflow(maximum, zero, &r));
    ASSERT_EQ(r, zero);
    ASSERT_FALSE(mul_overflow(maximum, one, &r));
    ASSERT_EQ(r, maximum);
    ASSERT_TRUE(mul_overflow(maximum, two, &r));
    ASSERT_TRUE(mul_overflow(maximum, half, &r));
    ASSERT_TRUE(mul_overflow(maximum, maximum, &r));
}
