// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/encoding/base64.hpp"
#include "ttauri/byte_string.hpp"
#include "ttauri/required.hpp"
#include "ttauri/exceptions.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;


TEST(base64, Encode) {
    ASSERT_EQ(encode_base64(to_bstring("")), "");
    ASSERT_EQ(encode_base64(to_bstring("f")), "Zg==");
    ASSERT_EQ(encode_base64(to_bstring("fo")), "Zm8=");
    ASSERT_EQ(encode_base64(to_bstring("foo")), "Zm9v");
    ASSERT_EQ(encode_base64(to_bstring("foob")), "Zm9vYg==");
    ASSERT_EQ(encode_base64(to_bstring("fooba")), "Zm9vYmE=");
    ASSERT_EQ(encode_base64(to_bstring("foobar")), "Zm9vYmFy");

    ASSERT_EQ(encode_base64(to_bstring("Hello World\n")), "SGVsbG8gV29ybGQK");
}

TEST(base64, Decode) {
    ASSERT_EQ(decode_base64(""), to_bstring(""));
    ASSERT_THROW(decode_base64("Z"), parse_error);
    ASSERT_EQ(decode_base64("Zg=="), to_bstring("f"));
    ASSERT_EQ(decode_base64("Zg="), to_bstring("f"));
    ASSERT_EQ(decode_base64("Zg"), to_bstring("f"));
    ASSERT_EQ(decode_base64("Zm8="), to_bstring("fo"));
    ASSERT_EQ(decode_base64("Zm8"), to_bstring("fo"));
    ASSERT_EQ(decode_base64("Zm9v"), to_bstring("foo"));
    ASSERT_THROW(decode_base64("Zm9vY"), parse_error);
    ASSERT_EQ(decode_base64("Zm9vYg=="), to_bstring("foob"));
    ASSERT_EQ(decode_base64("Zm9vYg="), to_bstring("foob"));
    ASSERT_EQ(decode_base64("Zm9vYg"), to_bstring("foob"));
    ASSERT_EQ(decode_base64("Zm9vYmE="), to_bstring("fooba"));
    ASSERT_EQ(decode_base64("Zm9vYmE"), to_bstring("fooba"));
    ASSERT_EQ(decode_base64("Zm9vYmFy"), to_bstring("foobar"));

    ASSERT_EQ(decode_base64("SGVsbG8gV29ybGQK"), to_bstring("Hello World\n"));
    ASSERT_EQ(decode_base64("SGVsb G8g\nV29ybGQK"), to_bstring("Hello World\n"));
    ASSERT_THROW(decode_base64("SGVsbG8g,V29ybGQK"), parse_error);
}
