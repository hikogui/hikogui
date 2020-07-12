// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/encoding/base64.hpp"
#include "ttauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;


TEST(base64, Encode) {
    ASSERT_EQ(base64_encode(to_bstring("")), "");
    ASSERT_EQ(base64_encode(to_bstring("f")), "Zg==");
    ASSERT_EQ(base64_encode(to_bstring("fo")), "Zm8=");
    ASSERT_EQ(base64_encode(to_bstring("foo")), "Zm9v");
    ASSERT_EQ(base64_encode(to_bstring("foob")), "Zm9vYg==");
    ASSERT_EQ(base64_encode(to_bstring("fooba")), "Zm9vYmE=");
    ASSERT_EQ(base64_encode(to_bstring("foobar")), "Zm9vYmFy");

    ASSERT_EQ(base64_encode(to_bstring("Hello World\n")), "SGVsbG8gV29ybGQK");
}

TEST(base64, Decode) {
    ASSERT_EQ(base64_decode(""), to_bstring(""));
    ASSERT_THROW(base64_decode("Z"), parse_error);
    ASSERT_EQ(base64_decode("Zg=="), to_bstring("f"));
    ASSERT_EQ(base64_decode("Zg="), to_bstring("f"));
    ASSERT_EQ(base64_decode("Zg"), to_bstring("f"));
    ASSERT_EQ(base64_decode("Zm8="), to_bstring("fo"));
    ASSERT_EQ(base64_decode("Zm8"), to_bstring("fo"));
    ASSERT_EQ(base64_decode("Zm9v"), to_bstring("foo"));
    ASSERT_THROW(base64_decode("Zm9vY"), parse_error);
    ASSERT_EQ(base64_decode("Zm9vYg=="), to_bstring("foob"));
    ASSERT_EQ(base64_decode("Zm9vYg="), to_bstring("foob"));
    ASSERT_EQ(base64_decode("Zm9vYg"), to_bstring("foob"));
    ASSERT_EQ(base64_decode("Zm9vYmE="), to_bstring("fooba"));
    ASSERT_EQ(base64_decode("Zm9vYmE"), to_bstring("fooba"));
    ASSERT_EQ(base64_decode("Zm9vYmFy"), to_bstring("foobar"));

    ASSERT_EQ(base64_decode("SGVsbG8gV29ybGQK"), to_bstring("Hello World\n"));
    ASSERT_EQ(base64_decode("SGVsb G8g\nV29ybGQK"), to_bstring("Hello World\n"));
    ASSERT_THROW(base64_decode("SGVsbG8g,V29ybGQK"), parse_error);
}
