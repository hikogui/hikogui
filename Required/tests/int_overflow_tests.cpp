// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Required/int_overflow.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>

using namespace std;
using namespace TTauri;

template <typename T>
class SignedIntOverflow : public ::testing::Test {
public:
    typedef std::list<T> List;
    static T shared_;
    T value_;
};

template <typename T>
class UnsignedIntOverflow : public ::testing::Test {
public:
    typedef std::list<T> List;
    static T shared_;
    T value_;
};

using SignedIntOverflow_Types = ::testing::Types<signed char, signed short, signed int, signed long, signed long long>;
TYPED_TEST_SUITE(SignedIntOverflow, SignedIntOverflow_Types);

using UnsignedIntOverflow_Types = ::testing::Types<unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;
TYPED_TEST_SUITE(UnsignedIntOverflow, UnsignedIntOverflow_Types);

TYPED_TEST(SignedIntOverflow, Add) {
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
    ASSERT_FALSE(add_overflow(minimum, zero, &r)); ASSERT_EQ(r, minimum);
    ASSERT_FALSE(add_overflow(minimum, one, &r)); ASSERT_EQ(r, low);
    ASSERT_FALSE(add_overflow(minimum, high, &r)); ASSERT_EQ(r, minTwo);
    ASSERT_FALSE(add_overflow(minimum, maximum, &r)); ASSERT_EQ(r, minOne);

    ASSERT_TRUE(add_overflow(low, minimum, &r));
    ASSERT_TRUE(add_overflow(low, low, &r));
    ASSERT_FALSE(add_overflow(low, minOne, &r)); ASSERT_EQ(r, minimum);
    ASSERT_FALSE(add_overflow(low, zero, &r)); ASSERT_EQ(r, low);
    ASSERT_FALSE(add_overflow(low, one, &r)); ASSERT_EQ(r, lessLow);
    ASSERT_FALSE(add_overflow(low, high, &r)); ASSERT_EQ(r, minOne);
    ASSERT_FALSE(add_overflow(low, maximum, &r)); ASSERT_EQ(r, zero);

    ASSERT_TRUE(add_overflow(minOne, minimum, &r));
    ASSERT_FALSE(add_overflow(minOne, low, &r)); ASSERT_EQ(r, minimum);
    ASSERT_FALSE(add_overflow(minOne, minOne, &r)); ASSERT_EQ(r, minTwo);
    ASSERT_FALSE(add_overflow(minOne, zero, &r)); ASSERT_EQ(r, minOne);
    ASSERT_FALSE(add_overflow(minOne, one, &r)); ASSERT_EQ(r, zero);
    ASSERT_FALSE(add_overflow(minOne, high, &r)); ASSERT_EQ(r, lessHigh);
    ASSERT_FALSE(add_overflow(minOne, maximum, &r)); ASSERT_EQ(r, high);

    ASSERT_FALSE(add_overflow(zero, minimum, &r)); ASSERT_EQ(r, minimum);
    ASSERT_FALSE(add_overflow(zero, low, &r)); ASSERT_EQ(r, low);
    ASSERT_FALSE(add_overflow(zero, minOne, &r)); ASSERT_EQ(r, minOne);
    ASSERT_FALSE(add_overflow(zero, zero, &r)); ASSERT_EQ(r, zero);
    ASSERT_FALSE(add_overflow(zero, one, &r)); ASSERT_EQ(r, one);
    ASSERT_FALSE(add_overflow(zero, high, &r)); ASSERT_EQ(r, high);
    ASSERT_FALSE(add_overflow(zero, maximum, &r)); ASSERT_EQ(r, maximum);

    ASSERT_FALSE(add_overflow(one, minimum, &r)); ASSERT_EQ(r, low);
    ASSERT_FALSE(add_overflow(one, low, &r)); ASSERT_EQ(r, lessLow);
    ASSERT_FALSE(add_overflow(one, minOne, &r)); ASSERT_EQ(r, zero);
    ASSERT_FALSE(add_overflow(one, zero, &r)); ASSERT_EQ(r, one);
    ASSERT_FALSE(add_overflow(one, one, &r)); ASSERT_EQ(r, two);
    ASSERT_FALSE(add_overflow(one, high, &r)); ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(one, maximum, &r));

    ASSERT_FALSE(add_overflow(high, minimum, &r)); ASSERT_EQ(r, minTwo);
    ASSERT_FALSE(add_overflow(high, low, &r)); ASSERT_EQ(r, minOne);
    ASSERT_FALSE(add_overflow(high, minOne, &r)); ASSERT_EQ(r, lessHigh);
    ASSERT_FALSE(add_overflow(high, zero, &r)); ASSERT_EQ(r, high);
    ASSERT_FALSE(add_overflow(high, one, &r)); ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(high, high, &r));
    ASSERT_TRUE(add_overflow(high, maximum, &r));

    ASSERT_FALSE(add_overflow(maximum, minimum, &r)); ASSERT_EQ(r, minOne);
    ASSERT_FALSE(add_overflow(maximum, low, &r)); ASSERT_EQ(r, zero);
    ASSERT_FALSE(add_overflow(maximum, minOne, &r)); ASSERT_EQ(r, high);
    ASSERT_FALSE(add_overflow(maximum, zero, &r)); ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(maximum, one, &r));
    ASSERT_TRUE(add_overflow(maximum, high, &r));
    ASSERT_TRUE(add_overflow(maximum, maximum, &r));
}


TYPED_TEST(UnsignedIntOverflow, Add) {
    TypeParam r;

    TypeParam zero = 0;
    TypeParam one = 1;
    TypeParam two = 2;
    TypeParam maximum = numeric_limits<TypeParam>::max();
    TypeParam high = maximum - 1;
    TypeParam lessHigh = maximum - 2;

    ASSERT_FALSE(add_overflow(zero, zero, &r)); ASSERT_EQ(r, zero);
    ASSERT_FALSE(add_overflow(zero, one, &r)); ASSERT_EQ(r, one);
    ASSERT_FALSE(add_overflow(zero, high, &r)); ASSERT_EQ(r, high);
    ASSERT_FALSE(add_overflow(zero, maximum, &r)); ASSERT_EQ(r, maximum);

    ASSERT_FALSE(add_overflow(one, zero, &r)); ASSERT_EQ(r, one);
    ASSERT_FALSE(add_overflow(one, one, &r)); ASSERT_EQ(r, two);
    ASSERT_FALSE(add_overflow(one, high, &r)); ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(one, maximum, &r));

    ASSERT_FALSE(add_overflow(high, zero, &r)); ASSERT_EQ(r, high);
    ASSERT_FALSE(add_overflow(high, one, &r)); ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(high, high, &r));
    ASSERT_TRUE(add_overflow(high, maximum, &r));

    ASSERT_FALSE(add_overflow(maximum, zero, &r)); ASSERT_EQ(r, maximum);
    ASSERT_TRUE(add_overflow(maximum, one, &r));
    ASSERT_TRUE(add_overflow(maximum, high, &r));
    ASSERT_TRUE(add_overflow(maximum, maximum, &r));
}

TYPED_TEST(SignedIntOverflow, Subtract) {
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

    ASSERT_TRUE(sub_overflow(minimum, minimum, &r));
    ASSERT_TRUE(sub_overflow(minimum, low, &r));
    ASSERT_TRUE(sub_overflow(minimum, minOne, &r));
    ASSERT_FALSE(sub_overflow(minimum, zero, &r)); ASSERT_EQ(r, minimum);
    ASSERT_FALSE(sub_overflow(minimum, one, &r)); ASSERT_EQ(r, low);
    ASSERT_FALSE(sub_overflow(minimum, high, &r)); ASSERT_EQ(r, minTwo);
    ASSERT_FALSE(sub_overflow(minimum, maximum, &r)); ASSERT_EQ(r, minOne);

    ASSERT_TRUE(sub_overflow(low, minimum, &r));
    ASSERT_TRUE(sub_overflow(low, low, &r));
    ASSERT_FALSE(sub_overflow(low, minOne, &r)); ASSERT_EQ(r, minimum);
    ASSERT_FALSE(sub_overflow(low, zero, &r)); ASSERT_EQ(r, low);
    ASSERT_FALSE(sub_overflow(low, one, &r)); ASSERT_EQ(r, lessLow);
    ASSERT_FALSE(sub_overflow(low, high, &r)); ASSERT_EQ(r, minOne);
    ASSERT_FALSE(sub_overflow(low, maximum, &r)); ASSERT_EQ(r, zero);

    ASSERT_TRUE(sub_overflow(minOne, minimum, &r));
    ASSERT_FALSE(sub_overflow(minOne, low, &r)); ASSERT_EQ(r, minimum);
    ASSERT_FALSE(sub_overflow(minOne, minOne, &r)); ASSERT_EQ(r, minTwo);
    ASSERT_FALSE(sub_overflow(minOne, zero, &r)); ASSERT_EQ(r, minOne);
    ASSERT_FALSE(sub_overflow(minOne, one, &r)); ASSERT_EQ(r, zero);
    ASSERT_FALSE(sub_overflow(minOne, high, &r)); ASSERT_EQ(r, lessHigh);
    ASSERT_FALSE(sub_overflow(minOne, maximum, &r)); ASSERT_EQ(r, high);

    ASSERT_FALSE(sub_overflow(zero, minimum, &r)); ASSERT_EQ(r, minimum);
    ASSERT_FALSE(sub_overflow(zero, low, &r)); ASSERT_EQ(r, low);
    ASSERT_FALSE(sub_overflow(zero, minOne, &r)); ASSERT_EQ(r, minOne);
    ASSERT_FALSE(sub_overflow(zero, zero, &r)); ASSERT_EQ(r, zero);
    ASSERT_FALSE(sub_overflow(zero, one, &r)); ASSERT_EQ(r, one);
    ASSERT_FALSE(sub_overflow(zero, high, &r)); ASSERT_EQ(r, high);
    ASSERT_FALSE(sub_overflow(zero, maximum, &r)); ASSERT_EQ(r, maximum);

    ASSERT_FALSE(sub_overflow(one, minimum, &r)); ASSERT_EQ(r, low);
    ASSERT_FALSE(sub_overflow(one, low, &r)); ASSERT_EQ(r, lessLow);
    ASSERT_FALSE(sub_overflow(one, minOne, &r)); ASSERT_EQ(r, zero);
    ASSERT_FALSE(sub_overflow(one, zero, &r)); ASSERT_EQ(r, one);
    ASSERT_FALSE(sub_overflow(one, one, &r)); ASSERT_EQ(r, two);
    ASSERT_FALSE(sub_overflow(one, high, &r)); ASSERT_EQ(r, maximum);
    ASSERT_TRUE(sub_overflow(one, maximum, &r));

    ASSERT_FALSE(sub_overflow(high, minimum, &r)); ASSERT_EQ(r, minTwo);
    ASSERT_FALSE(sub_overflow(high, low, &r)); ASSERT_EQ(r, minOne);
    ASSERT_FALSE(sub_overflow(high, minOne, &r)); ASSERT_EQ(r, lessHigh);
    ASSERT_FALSE(sub_overflow(high, zero, &r)); ASSERT_EQ(r, high);
    ASSERT_FALSE(sub_overflow(high, one, &r)); ASSERT_EQ(r, maximum);
    ASSERT_TRUE(sub_overflow(high, high, &r));
    ASSERT_TRUE(sub_overflow(high, maximum, &r));

    ASSERT_FALSE(sub_overflow(maximum, minimum, &r)); ASSERT_EQ(r, minOne);
    ASSERT_FALSE(sub_overflow(maximum, low, &r)); ASSERT_EQ(r, zero);
    ASSERT_FALSE(sub_overflow(maximum, minOne, &r)); ASSERT_EQ(r, high);
    ASSERT_FALSE(sub_overflow(maximum, zero, &r)); ASSERT_EQ(r, maximum);
    ASSERT_TRUE(sub_overflow(maximum, one, &r));
    ASSERT_TRUE(sub_overflow(maximum, high, &r));
    ASSERT_TRUE(sub_overflow(maximum, maximum, &r));
}
