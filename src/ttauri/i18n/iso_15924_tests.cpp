// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/i18n/iso_15924.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

TEST(iso_15925, from_code4)
{
    ASSERT_EQ(tt::iso_15924{"Latn"}, tt::iso_15924{215});
    ASSERT_EQ(tt::iso_15924{"Yiii"}, tt::iso_15924{460});
    ASSERT_THROW(tt::iso_15924{"yi  "}, tt::parse_error);
    ASSERT_THROW(tt::iso_15924{"Foob"}, tt::parse_error);
}

TEST(iso_15925, from_unicode)
{
    ASSERT_EQ(tt::iso_15924{tt::unicode_script::Latin}, tt::iso_15924{215});
    ASSERT_EQ(tt::iso_15924{tt::unicode_script::Yi}, tt::iso_15924{460});
}

TEST(iso_15925, to_code4)
{
    ASSERT_EQ(tt::iso_15924{215}.code4(), "Latn");
    ASSERT_EQ(tt::iso_15924{460}.code4(), "Yiii");
}

TEST(iso_15925, to_code4_open_type)
{
    ASSERT_EQ(tt::iso_15924{215}.code4_open_type(), "latn");
    ASSERT_EQ(tt::iso_15924{460}.code4_open_type(), "yi  ");
}

//TEST(iso_15925, to_code4_unicode_script)
//{
//    ASSERT_EQ(tt::iso_15924{215}.unicode_script(), tt::unicode_script::Latin);
//    ASSERT_EQ(tt::iso_15924{460}.unicode_script(), tt::unicode_script::Yi);
//}
