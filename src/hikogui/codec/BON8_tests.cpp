// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "BON8.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>



using namespace std;
using namespace hi;

TEST(BON8, encode_positive_integers)
{
    ASSERT_EQ(encode_BON8(datum{0}), to_bstring(0x90));
    ASSERT_EQ(encode_BON8(datum{39}), to_bstring(0xb7));
    ASSERT_EQ(encode_BON8(datum{40}), to_bstring(0xc2, 0x00));
    ASSERT_EQ(encode_BON8(datum{3879}), to_bstring(0xdf, 0x7f));
    ASSERT_EQ(encode_BON8(datum{3880}), to_bstring(0xe0, 0x00, 0x00));
    ASSERT_EQ(encode_BON8(datum{528167}), to_bstring(0xef, 0x7f, 0xff));
    ASSERT_EQ(encode_BON8(datum{528168}), to_bstring(0xf0, 0x00, 0x00, 0x00));
    ASSERT_EQ(encode_BON8(datum{67637031}), to_bstring(0xf7, 0x7f, 0xff, 0xff));
    ASSERT_EQ(encode_BON8(datum{67637032}), to_bstring(0x8c, 0x04, 0x08, 0x0f, 0x28));
    ASSERT_EQ(encode_BON8(datum{2147483647}), to_bstring(0x8c, 0x7f, 0xff, 0xff, 0xff));
    ASSERT_EQ(encode_BON8(datum{2147483648LL}), to_bstring(0x8d, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00));
    ASSERT_EQ(encode_BON8(datum{9223372036854775807LL}), to_bstring(0x8d, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff));
}

TEST(BON8, decode_positive_integers)
{
    ASSERT_EQ(datum{0}, decode_BON8(to_bstring(0x90)));
    ASSERT_EQ(datum{39}, decode_BON8(to_bstring(0xb7)));
    ASSERT_EQ(datum{40}, decode_BON8(to_bstring(0xc2, 0x00)));
    ASSERT_EQ(datum{3879}, decode_BON8(to_bstring(0xdf, 0x7f)));
    ASSERT_EQ(datum{3880}, decode_BON8(to_bstring(0xe0, 0x00, 0x00)));
    ASSERT_EQ(datum{528167}, decode_BON8(to_bstring(0xef, 0x7f, 0xff)));
    ASSERT_EQ(datum{528168}, decode_BON8(to_bstring(0xf0, 0x00, 0x00, 0x00)));
    ASSERT_EQ(datum{67637031}, decode_BON8(to_bstring(0xf7, 0x7f, 0xff, 0xff)));
    ASSERT_EQ(datum{67637032}, decode_BON8(to_bstring(0x8c, 0x04, 0x08, 0x0f, 0x28)));
    ASSERT_EQ(datum{2147483647}, decode_BON8(to_bstring(0x8c, 0x7f, 0xff, 0xff, 0xff)));
    ASSERT_EQ(datum{2147483648LL}, decode_BON8(to_bstring(0x8d, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00)));
    ASSERT_EQ(datum{9223372036854775807LL}, decode_BON8(to_bstring(0x8d, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff)));
}

TEST(BON8, encode_negative_integers)
{
    ASSERT_EQ(encode_BON8(datum{-1}), to_bstring(0xb8));
    ASSERT_EQ(encode_BON8(datum{-10}), to_bstring(0xc1));
    ASSERT_EQ(encode_BON8(datum{-11}), to_bstring(0xc2, 0xc0));
    ASSERT_EQ(encode_BON8(datum{-1930}), to_bstring(0xdf, 0xff));
    ASSERT_EQ(encode_BON8(datum{-1931}), to_bstring(0xe0, 0xc0, 0x00));
    ASSERT_EQ(encode_BON8(datum{-264074}), to_bstring(0xef, 0xff, 0xff));
    ASSERT_EQ(encode_BON8(datum{-264075}), to_bstring(0xf0, 0xc0, 0x00, 0x00));
    ASSERT_EQ(encode_BON8(datum{-33818506}), to_bstring(0xf7, 0xff, 0xff, 0xff));
    ASSERT_EQ(encode_BON8(datum{-33818507}), to_bstring(0x8c, 0xfd, 0xfb, 0xf8, 0x75));
    ASSERT_EQ(encode_BON8(datum{-2147483648LL}), to_bstring(0x8c, 0x80, 0x00, 0x00, 0x00));
    ASSERT_EQ(encode_BON8(datum{-2147483649LL}), to_bstring(0x8d, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff));
    ASSERT_EQ(
        encode_BON8(datum{std::numeric_limits<int64_t>::min()}),
        to_bstring(0x8d, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00));
}

TEST(BON8, decode_negative_integers)
{
    ASSERT_EQ(datum{-1}, decode_BON8(to_bstring(0xb8)));
    ASSERT_EQ(datum{-10}, decode_BON8(to_bstring(0xc1)));
    ASSERT_EQ(datum{-11}, decode_BON8(to_bstring(0xc2, 0xc0)));
    ASSERT_EQ(datum{-1930}, decode_BON8(to_bstring(0xdf, 0xff)));
    ASSERT_EQ(datum{-1931}, decode_BON8(to_bstring(0xe0, 0xc0, 0x00)));
    ASSERT_EQ(datum{-264074}, decode_BON8(to_bstring(0xef, 0xff, 0xff)));
    ASSERT_EQ(datum{-264075}, decode_BON8(to_bstring(0xf0, 0xc0, 0x00, 0x00)));
    ASSERT_EQ(datum{-33818506}, decode_BON8(to_bstring(0xf7, 0xff, 0xff, 0xff)));
    ASSERT_EQ(datum{-33818507}, decode_BON8(to_bstring(0x8c, 0xfd, 0xfb, 0xf8, 0x75)));
    ASSERT_EQ(datum{-2147483648LL}, decode_BON8(to_bstring(0x8c, 0x80, 0x00, 0x00, 0x00)));
    ASSERT_EQ(datum{-2147483649LL}, decode_BON8(to_bstring(0x8d, 0xff, 0xff, 0xff, 0xff, 0x7f, 0xff, 0xff, 0xff)));
    ASSERT_EQ(
        datum{std::numeric_limits<int64_t>::min()},
        decode_BON8(to_bstring(0x8d, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)));
}
