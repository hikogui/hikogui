// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "spreadsheet_address.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace hi;

TEST(spreadsheet_address, parse_absolute_spreadsheet_address)
{
    ASSERT_EQ(parse_spreadsheet_address("A1"), std::pair(0_uz, 0_uz));
    ASSERT_EQ(parse_spreadsheet_address("A9"), std::pair(0_uz, 8_uz));
    ASSERT_EQ(parse_spreadsheet_address("A09"), std::pair(0_uz, 8_uz));
    ASSERT_EQ(parse_spreadsheet_address("A10"), std::pair(0_uz, 9_uz));

    ASSERT_EQ(parse_spreadsheet_address("a1"), std::pair(0_uz, 0_uz));
    ASSERT_EQ(parse_spreadsheet_address("a9"), std::pair(0_uz, 8_uz));
    ASSERT_EQ(parse_spreadsheet_address("a09"), std::pair(0_uz, 8_uz));
    ASSERT_EQ(parse_spreadsheet_address("a10"), std::pair(0_uz, 9_uz));

    ASSERT_EQ(parse_spreadsheet_address("B1"), std::pair(1_uz, 0_uz));
    ASSERT_EQ(parse_spreadsheet_address("B9"), std::pair(1_uz, 8_uz));
    ASSERT_EQ(parse_spreadsheet_address("B09"), std::pair(1_uz, 8_uz));
    ASSERT_EQ(parse_spreadsheet_address("B10"), std::pair(1_uz, 9_uz));

    ASSERT_EQ(parse_spreadsheet_address("Z1"), std::pair(25_uz, 0_uz));
    ASSERT_EQ(parse_spreadsheet_address("Z9"), std::pair(25_uz, 8_uz));
    ASSERT_EQ(parse_spreadsheet_address("Z09"), std::pair(25_uz, 8_uz));
    ASSERT_EQ(parse_spreadsheet_address("Z10"), std::pair(25_uz, 9_uz));

    ASSERT_EQ(parse_spreadsheet_address("AA1"), std::pair(26_uz, 0_uz));
    ASSERT_EQ(parse_spreadsheet_address("AA9"), std::pair(26_uz, 8_uz));
    ASSERT_EQ(parse_spreadsheet_address("AA09"), std::pair(26_uz, 8_uz));
    ASSERT_EQ(parse_spreadsheet_address("AA10"), std::pair(26_uz, 9_uz));
}
