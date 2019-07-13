// Copyright 2019 Pokitec
// All rights reserved.

#include "pickle.hpp"
#include "required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;
using namespace std::literals;

TEST(PickleTests, Integers) {
    ASSERT_EQ((""s ^ 0x00), "\x00"s);
    ASSERT_EQ((""s ^ 0x6f), "\x6f"s);
    ASSERT_EQ((""s ^ 0x70), "\xf0\x00"s);
    ASSERT_EQ((""s ^ 0x1fff), "\xff\x3f"s);
    ASSERT_EQ((""s ^ 0x3fff), "\xff\xff\x00"s); // Sign-bit causes overflow to three bytes.
}

TEST(PickleTests, Strings) {
    ASSERT_EQ((""s ^ "hello"), "\x79\x05hello"s);

    char const *p = "hello";
    ASSERT_EQ((""s ^ p), "\x79\x05hello"s);

    ASSERT_EQ((""s ^ u8"h\U0001f34c" "llo"), u8"\x79\x08h\U0001f34c" "llo"s);
}
