// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "BON8.hpp"
#include "../utility/utility.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(BON8_suite) {

TEST_CASE(encode_positive_integers)
{
    REQUIRE(hi::encode_BON8(hi::datum{0}) == hi::to_bstring(0x90));
    REQUIRE(hi::encode_BON8(hi::datum{39}) == hi::to_bstring(0xb7));
    REQUIRE(hi::encode_BON8(hi::datum{40}) == hi::to_bstring(0xc2, 0x00));
    REQUIRE(hi::encode_BON8(hi::datum{3879}) == hi::to_bstring(0xdf, 0x7f));
    REQUIRE(hi::encode_BON8(hi::datum{3880}) == hi::to_bstring(0xe0, 0x00, 0x00));
    REQUIRE(hi::encode_BON8(hi::datum{528167}) == hi::to_bstring(0xef, 0x7f, 0xff));
    REQUIRE(hi::encode_BON8(hi::datum{528168}) == hi::to_bstring(0xf0, 0x00, 0x00, 0x00));
    REQUIRE(hi::encode_BON8(hi::datum{67637031}) == hi::to_bstring(0xf7, 0x7f, 0xff, 0xff));
    REQUIRE(hi::encode_BON8(hi::datum{67637032}) == hi::to_bstring(0x8c, 0x04, 0x08, 0x0f, 0x28));
    REQUIRE(hi::encode_BON8(hi::datum{2147483647}) == hi::to_bstring(0x8c, 0x7f, 0xff, 0xff, 0xff));
    REQUIRE(hi::encode_BON8(hi::datum{2147483648LL}) == hi::to_bstring(0x8d, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00));
    REQUIRE(hi::encode_BON8(hi::datum{9223372036854775807LL}) == hi::to_bstring(0x8d, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff));
}

TEST_CASE(decode_positive_integers)
{
    REQUIRE(hi::datum{0} == hi::decode_BON8(hi::to_bstring(0x90)));
    REQUIRE(hi::datum{39} == hi::decode_BON8(hi::to_bstring(0xb7)));
    REQUIRE(hi::datum{40} == hi::decode_BON8(hi::to_bstring(0xc2, 0x00)));
    REQUIRE(hi::datum{3879} == hi::decode_BON8(hi::to_bstring(0xdf, 0x7f)));
    REQUIRE(hi::datum{3880} == hi::decode_BON8(hi::to_bstring(0xe0, 0x00, 0x00)));
    REQUIRE(hi::datum{528167} == hi::decode_BON8(hi::to_bstring(0xef, 0x7f, 0xff)));
    REQUIRE(hi::datum{528168} == hi::decode_BON8(hi::to_bstring(0xf0, 0x00, 0x00, 0x00)));
    REQUIRE(hi::datum{67637031} == hi::decode_BON8(hi::to_bstring(0xf7, 0x7f, 0xff, 0xff)));
    REQUIRE(hi::datum{67637032} == hi::decode_BON8(hi::to_bstring(0x8c, 0x04, 0x08, 0x0f, 0x28)));
    REQUIRE(hi::datum{2147483647} == hi::decode_BON8(hi::to_bstring(0x8c, 0x7f, 0xff, 0xff, 0xff)));
    REQUIRE(hi::datum{2147483648LL} == hi::decode_BON8(hi::to_bstring(0x8d, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00)));
    REQUIRE(hi::datum{9223372036854775807LL} == hi::decode_BON8(hi::to_bstring(0x8d, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff)));
}

TEST_CASE(encode_negative_integers)
{
    REQUIRE(hi::encode_BON8(hi::datum{-1}) == hi::to_bstring(0xb8));
    REQUIRE(hi::encode_BON8(hi::datum{-10}) == hi::to_bstring(0xc1));
    REQUIRE(hi::encode_BON8(hi::datum{-11}) == hi::to_bstring(0xc2, 0xc0));
    REQUIRE(hi::encode_BON8(hi::datum{-1930}) == hi::to_bstring(0xdf, 0xff));
    REQUIRE(hi::encode_BON8(hi::datum{-1931}) == hi::to_bstring(0xe0, 0xc0, 0x00));
    REQUIRE(hi::encode_BON8(hi::datum{-264074}) == hi::to_bstring(0xef, 0xff, 0xff));
    REQUIRE(hi::encode_BON8(hi::datum{-264075}) == hi::to_bstring(0xf0, 0xc0, 0x00, 0x00));
    REQUIRE(hi::encode_BON8(hi::datum{-33818506}) == hi::to_bstring(0xf7, 0xff, 0xff, 0xff));
    REQUIRE(hi::encode_BON8(hi::datum{-33818507}) == hi::to_bstring(0x8c, 0xfd, 0xfb, 0xf8, 0x75));
    REQUIRE(hi::encode_BON8(hi::datum{-2147483648LL}) == hi::to_bstring(0x8c, 0x80, 0x00, 0x00, 0x00));
    REQUIRE(hi::encode_BON8(hi::datum{-2147483649LL}) == hi::to_bstring(0x8d, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff));
    REQUIRE(
        hi::encode_BON8(hi::datum{std::numeric_limits<int64_t>::min()}) ==
        hi::to_bstring(0x8d, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));
}

TEST_CASE(decode_negative_integers)
{
    REQUIRE(hi::datum{-1} == hi::decode_BON8(hi::to_bstring(0xb8)));
    REQUIRE(hi::datum{-10} == hi::decode_BON8(hi::to_bstring(0xc1)));
    REQUIRE(hi::datum{-11} == hi::decode_BON8(hi::to_bstring(0xc2, 0xc0)));
    REQUIRE(hi::datum{-1930} == hi::decode_BON8(hi::to_bstring(0xdf, 0xff)));
    REQUIRE(hi::datum{-1931} == hi::decode_BON8(hi::to_bstring(0xe0, 0xc0, 0x00)));
    REQUIRE(hi::datum{-264074} == hi::decode_BON8(hi::to_bstring(0xef, 0xff, 0xff)));
    REQUIRE(hi::datum{-264075} == hi::decode_BON8(hi::to_bstring(0xf0, 0xc0, 0x00, 0x00)));
    REQUIRE(hi::datum{-33818506} == hi::decode_BON8(hi::to_bstring(0xf7, 0xff, 0xff, 0xff)));
    REQUIRE(hi::datum{-33818507} == hi::decode_BON8(hi::to_bstring(0x8c, 0xfd, 0xfb, 0xf8, 0x75)));
    REQUIRE(hi::datum{-2147483648LL} == hi::decode_BON8(hi::to_bstring(0x8c, 0x80, 0x00, 0x00, 0x00)));
    REQUIRE(hi::datum{-2147483649LL} == hi::decode_BON8(hi::to_bstring(0x8d, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff)));
    REQUIRE(
        hi::datum{std::numeric_limits<int64_t>::min()} ==
        hi::decode_BON8(hi::to_bstring(0x8d, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)));
}

}; // TEST_SUITE(BON8_suite)
