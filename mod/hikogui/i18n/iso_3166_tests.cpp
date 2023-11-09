// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_3166.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

TEST(iso_3166, parse)
{
    ASSERT_EQ(hi::iso_3166("NL").number(), 528);
    ASSERT_EQ(hi::iso_3166("Nl").number(), 528);
    ASSERT_EQ(hi::iso_3166("nl").number(), 528);
    ASSERT_EQ(hi::iso_3166("NLD").number(), 528);
    ASSERT_EQ(hi::iso_3166("Nld").number(), 528);
    ASSERT_EQ(hi::iso_3166("nld").number(), 528);
    ASSERT_EQ(hi::iso_3166("528").number(), 528);

    ASSERT_EQ(hi::iso_3166("BE").number(), 56);
    ASSERT_EQ(hi::iso_3166("Be").number(), 56);
    ASSERT_EQ(hi::iso_3166("be").number(), 56);
    ASSERT_EQ(hi::iso_3166("BEL").number(), 56);
    ASSERT_EQ(hi::iso_3166("Bel").number(), 56);
    ASSERT_EQ(hi::iso_3166("bel").number(), 56);
    ASSERT_EQ(hi::iso_3166("056").number(), 56);
    ASSERT_EQ(hi::iso_3166("56").number(), 56);

    ASSERT_THROW(hi::iso_3166(""), hi::parse_error);
    ASSERT_THROW(hi::iso_3166("x"), hi::parse_error);
    ASSERT_THROW(hi::iso_3166("xx"), hi::parse_error);
    ASSERT_THROW(hi::iso_3166("xxx"), hi::parse_error);
    ASSERT_THROW(hi::iso_3166("xxxx"), hi::parse_error);

    ASSERT_EQ(hi::iso_3166("0").number(), 0);
    ASSERT_TRUE(hi::iso_3166("0").empty());
    ASSERT_EQ(hi::iso_3166("1").number(), 1);
    ASSERT_EQ(hi::iso_3166("01").number(), 1);
    ASSERT_EQ(hi::iso_3166("10").number(), 10);
    ASSERT_EQ(hi::iso_3166("010").number(), 10);
    ASSERT_EQ(hi::iso_3166("100").number(), 100);
    ASSERT_EQ(hi::iso_3166("0100").number(), 100);
    ASSERT_EQ(hi::iso_3166("999").number(), 999);
    ASSERT_THROW(hi::iso_3166("1000"), hi::parse_error);
    ASSERT_THROW(hi::iso_3166("-1"), hi::parse_error);
}

TEST(iso_3166, code2)
{
    ASSERT_EQ(hi::iso_3166(528).code2(), "NL");
    ASSERT_EQ(hi::iso_3166(56).code2(), "BE");
    ASSERT_EQ(hi::iso_3166(999).code2(), "ZZ");
}

TEST(iso_3166, code3)
{
    ASSERT_EQ(hi::iso_3166(528).code3(), "NLD");
    ASSERT_EQ(hi::iso_3166(56).code3(), "BEL");
    ASSERT_EQ(hi::iso_3166(999).code3(), "ZZZ");
}
