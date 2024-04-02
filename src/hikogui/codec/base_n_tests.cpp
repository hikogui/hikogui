// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "base_n.hpp"
#include "../container/container.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(base_n_suite) {

TEST_CASE(base16_encode)
{
    REQUIRE(hi::base16::encode(hi::to_bstring("")) == "");
    REQUIRE(hi::base16::encode(hi::to_bstring("f")) == "66");
    REQUIRE(hi::base16::encode(hi::to_bstring("fo")) == "666F");
    REQUIRE(hi::base16::encode(hi::to_bstring("foo")) == "666F6F");
    REQUIRE(hi::base16::encode(hi::to_bstring("foob")) == "666F6F62");
    REQUIRE(hi::base16::encode(hi::to_bstring("fooba")) == "666F6F6261");
    REQUIRE(hi::base16::encode(hi::to_bstring("foobar")) == "666F6F626172");
}

TEST_CASE(base64_encode)
{
    REQUIRE(hi::base64::encode(hi::to_bstring("")) == "");
    REQUIRE(hi::base64::encode(hi::to_bstring("f")) == "Zg==");
    REQUIRE(hi::base64::encode(hi::to_bstring("fo")) == "Zm8=");
    REQUIRE(hi::base64::encode(hi::to_bstring("foo")) == "Zm9v");
    REQUIRE(hi::base64::encode(hi::to_bstring("foob")) == "Zm9vYg==");
    REQUIRE(hi::base64::encode(hi::to_bstring("fooba")) == "Zm9vYmE=");
    REQUIRE(hi::base64::encode(hi::to_bstring("foobar")) == "Zm9vYmFy");

    REQUIRE(hi::base64::encode(hi::to_bstring("Hello World\n")) == "SGVsbG8gV29ybGQK");
}

TEST_CASE(base64_decode)
{
    REQUIRE(hi::base64::decode("") == hi::to_bstring(""));
    REQUIRE_THROWS(hi::base64::decode("Z"), hi::parse_error);
    REQUIRE(hi::base64::decode("Zg==") == hi::to_bstring("f"));
    REQUIRE(hi::base64::decode("Zg=") == hi::to_bstring("f"));
    REQUIRE(hi::base64::decode("Zg") == hi::to_bstring("f"));
    REQUIRE(hi::base64::decode("Zm8=") == hi::to_bstring("fo"));
    REQUIRE(hi::base64::decode("Zm8") == hi::to_bstring("fo"));
    REQUIRE(hi::base64::decode("Zm9v") == hi::to_bstring("foo"));
    REQUIRE_THROWS(hi::base64::decode("Zm9vY"), hi::parse_error);
    REQUIRE(hi::base64::decode("Zm9vYg==") == hi::to_bstring("foob"));
    REQUIRE(hi::base64::decode("Zm9vYg=") == hi::to_bstring("foob"));
    REQUIRE(hi::base64::decode("Zm9vYg") == hi::to_bstring("foob"));
    REQUIRE(hi::base64::decode("Zm9vYmE=") == hi::to_bstring("fooba"));
    REQUIRE(hi::base64::decode("Zm9vYmE") == hi::to_bstring("fooba"));
    REQUIRE(hi::base64::decode("Zm9vYmFy") == hi::to_bstring("foobar"));

    REQUIRE(hi::base64::decode("SGVsbG8gV29ybGQK") == hi::to_bstring("Hello World\n"));
    REQUIRE(hi::base64::decode("SGVsb G8g\nV29ybGQK") == hi::to_bstring("Hello World\n"));
    REQUIRE_THROWS(hi::base64::decode("SGVsbG8g,V29ybGQK"), hi::parse_error);
}

};
