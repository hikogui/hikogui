// Copyright 2019 Pokitec
// All rights reserved.

#include "BinaryKey.hpp"
#include "required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(BinaryKeyTests, createBinaryKeys) {
    ASSERT_EQ(BinaryKey("foo").data, "\x0d" "foo");
}
