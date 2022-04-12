// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/i18n/iso_3166.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

TEST(iso_3166, parse)
{
    ASSERT_EQ(tt::iso_3166("NL").number(), 528);
    ASSERT_EQ(tt::iso_3166("Nl").number(), 528);
    ASSERT_EQ(tt::iso_3166("nl").number(), 528);
    ASSERT_EQ(tt::iso_3166("NLD").number(), 528);
    ASSERT_EQ(tt::iso_3166("Nld").number(), 528);
    ASSERT_EQ(tt::iso_3166("nld").number(), 528);
    ASSERT_EQ(tt::iso_3166("528").number(), 528);

    ASSERT_EQ(tt::iso_3166("BE").number(), 56);
    ASSERT_EQ(tt::iso_3166("Be").number(), 56);
    ASSERT_EQ(tt::iso_3166("be").number(), 56);
    ASSERT_EQ(tt::iso_3166("BEL").number(), 56);
    ASSERT_EQ(tt::iso_3166("Bel").number(), 56);
    ASSERT_EQ(tt::iso_3166("bel").number(), 56);
    ASSERT_EQ(tt::iso_3166("056").number(), 56);
    ASSERT_EQ(tt::iso_3166("56").number(), 56);

    ASSERT_THROW(tt::iso_3166(""), tt::parse_error);
    ASSERT_THROW(tt::iso_3166("x"), tt::parse_error);
    ASSERT_THROW(tt::iso_3166("xx"), tt::parse_error);
    ASSERT_THROW(tt::iso_3166("xxx"), tt::parse_error);
    ASSERT_THROW(tt::iso_3166("xxxx"), tt::parse_error);

    ASSERT_EQ(tt::iso_3166("0").number(), 0);
    ASSERT_EQ(tt::iso_3166("1").number(), 1);
    ASSERT_EQ(tt::iso_3166("01").number(), 1);
    ASSERT_EQ(tt::iso_3166("10").number(), 10);
    ASSERT_EQ(tt::iso_3166("010").number(), 10);
    ASSERT_EQ(tt::iso_3166("100").number(), 100);
    ASSERT_EQ(tt::iso_3166("0100").number(), 100);
    ASSERT_EQ(tt::iso_3166("999").number(), 999);
    ASSERT_EQ(tt::iso_3166("999").empty(), true);
    ASSERT_THROW(tt::iso_3166("1000"), tt::parse_error);
    ASSERT_THROW(tt::iso_3166("-1"), tt::parse_error);
}

TEST(iso_3166, code2)
{
    ASSERT_EQ(tt::iso_3166(528).code2(), "NL");
    ASSERT_EQ(tt::iso_3166(56).code2(), "BE");
    ASSERT_EQ(tt::iso_3166(999).code2(), "ZZ");
}

TEST(iso_3166, code3)
{
    ASSERT_EQ(tt::iso_3166(528).code3(), "NLD");
    ASSERT_EQ(tt::iso_3166(56).code3(), "BEL");
    ASSERT_EQ(tt::iso_3166(999).code3(), "ZZZ");
}
