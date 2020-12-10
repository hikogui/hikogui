// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/encoding/base16.hpp"
#include "ttauri/required.hpp"
#include "ttauri/exception.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;


TEST(base16, Encode) {
    ASSERT_EQ(encode_base16(to_bstring("")), "");
    ASSERT_EQ(encode_base16(to_bstring("f")), "66");
    ASSERT_EQ(encode_base16(to_bstring("fo")), "666F");
    ASSERT_EQ(encode_base16(to_bstring("foo")), "666F6F");
    ASSERT_EQ(encode_base16(to_bstring("foob")), "666F6F62");
    ASSERT_EQ(encode_base16(to_bstring("fooba")), "666F6F6261");
    ASSERT_EQ(encode_base16(to_bstring("foobar")), "666F6F626172");
}

