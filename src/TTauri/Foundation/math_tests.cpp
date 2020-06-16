// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/math.hpp"
#include "TTauri/Foundation/required.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;

TEST(Math, NextPowerOfTwo) {
    ASSERT_EQ(next_power_of_two(15), 16);
    ASSERT_EQ(next_power_of_two(16), 16);
    ASSERT_EQ(next_power_of_two(17), 32);

}
