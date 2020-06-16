// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/safe_int.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <limits>

using namespace std;
using namespace tt;


TEST(SafeIntTests, Add) {
    tint64_t r;

    //ttlet zero = tint64_t{0};
    //ttlet one = tint64_t{1};
    //ttlet minTwo = tint64_t{-2};
    //ttlet minOne = tint64_t{-1};
    //ttlet two = tint64_t{2};
    //ttlet maximum = tint64_t{numeric_limits<int64_t>::max()};
    //ttlet high = tint64_t{maximum - 1};
    //ttlet lessHigh = tint64_t{maximum - 2};
    ttlet minimum = tint64_t{numeric_limits<int64_t>::min()};
    //ttlet low = tint64_t{minimum + 1};
    //ttlet lessLow = tint64_t{minimum + 2};

    ASSERT_THROW(r = minimum + minimum, tt::math_error);

    /*ASSERT_TRUE(add_overflow(minimum, minimum, &r));
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
    ASSERT_TRUE(add_overflow(maximum, maximum, &r));*/
}

