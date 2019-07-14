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
    ASSERT_EQ(pickle(0x00), "\x80"s);
    ASSERT_EQ(pickle(0x3f), "\xbf"s);
    ASSERT_EQ(pickle(0x40), "\x40\x80"s); // Sign-bit causes overflow to two bytes.
    ASSERT_EQ(pickle(0x7f), "\x7f\x80"s);
    ASSERT_EQ(pickle(0x80), "\x00\x81"s);
    ASSERT_EQ(pickle(0x1fff), "\x7f\xbf"s);
    ASSERT_EQ(pickle(0x3fff), "\x7f\x7f\x80"s); // Sign-bit causes overflow to three bytes.

    ASSERT_EQ(pickle(-1), "\x7f\xff"s);

}

TEST(PickleTests, Strings) {
    char const *p = "hello";
    ASSERT_EQ(pickle(p), "\xc5hello");

    ASSERT_EQ(pickle(""), "\xc0");
    ASSERT_EQ(pickle("hello"), "\xc5hello");
    ASSERT_EQ(pickle(u8"h\U0001f34c" "llo"), "\xc8h\xf0\x9f\x8d\x8cllo");

    ASSERT_EQ(pickle("The quick brown fox jumps over the lazy dog."), "\xfb\xacThe quick brown fox jumps over the lazy dog.");
}
