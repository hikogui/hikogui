// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/codec/base_n.hpp"
#include "hikogui/byte_string.hpp"
#include "hikogui/utility.hpp"
#include "hikogui/exception.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace hi;

TEST(base_n, base16_encode)
{
    ASSERT_EQ(base16::encode(to_bstring("")), "");
    ASSERT_EQ(base16::encode(to_bstring("f")), "66");
    ASSERT_EQ(base16::encode(to_bstring("fo")), "666F");
    ASSERT_EQ(base16::encode(to_bstring("foo")), "666F6F");
    ASSERT_EQ(base16::encode(to_bstring("foob")), "666F6F62");
    ASSERT_EQ(base16::encode(to_bstring("fooba")), "666F6F6261");
    ASSERT_EQ(base16::encode(to_bstring("foobar")), "666F6F626172");
}

TEST(base_n, base64_encode)
{
    ASSERT_EQ(base64::encode(to_bstring("")), "");
    ASSERT_EQ(base64::encode(to_bstring("f")), "Zg==");
    ASSERT_EQ(base64::encode(to_bstring("fo")), "Zm8=");
    ASSERT_EQ(base64::encode(to_bstring("foo")), "Zm9v");
    ASSERT_EQ(base64::encode(to_bstring("foob")), "Zm9vYg==");
    ASSERT_EQ(base64::encode(to_bstring("fooba")), "Zm9vYmE=");
    ASSERT_EQ(base64::encode(to_bstring("foobar")), "Zm9vYmFy");

    ASSERT_EQ(base64::encode(to_bstring("Hello World\n")), "SGVsbG8gV29ybGQK");
}

TEST(base_n, base64_decode)
{
    ASSERT_EQ(base64::decode(""), to_bstring(""));
    ASSERT_THROW(base64::decode("Z"), parse_error);
    ASSERT_EQ(base64::decode("Zg=="), to_bstring("f"));
    ASSERT_EQ(base64::decode("Zg="), to_bstring("f"));
    ASSERT_EQ(base64::decode("Zg"), to_bstring("f"));
    ASSERT_EQ(base64::decode("Zm8="), to_bstring("fo"));
    ASSERT_EQ(base64::decode("Zm8"), to_bstring("fo"));
    ASSERT_EQ(base64::decode("Zm9v"), to_bstring("foo"));
    ASSERT_THROW(base64::decode("Zm9vY"), parse_error);
    ASSERT_EQ(base64::decode("Zm9vYg=="), to_bstring("foob"));
    ASSERT_EQ(base64::decode("Zm9vYg="), to_bstring("foob"));
    ASSERT_EQ(base64::decode("Zm9vYg"), to_bstring("foob"));
    ASSERT_EQ(base64::decode("Zm9vYmE="), to_bstring("fooba"));
    ASSERT_EQ(base64::decode("Zm9vYmE"), to_bstring("fooba"));
    ASSERT_EQ(base64::decode("Zm9vYmFy"), to_bstring("foobar"));

    ASSERT_EQ(base64::decode("SGVsbG8gV29ybGQK"), to_bstring("Hello World\n"));
    ASSERT_EQ(base64::decode("SGVsb G8g\nV29ybGQK"), to_bstring("Hello World\n"));
    ASSERT_THROW(base64::decode("SGVsbG8g,V29ybGQK"), parse_error);
}
