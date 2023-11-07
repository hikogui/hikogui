// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_15924.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

TEST(iso_15925, from_code4)
{
    ASSERT_EQ(hi::iso_15924{"Latn"}.number(), 215);
    ASSERT_EQ(hi::iso_15924{"LATN"}.number(), 215);
    ASSERT_EQ(hi::iso_15924{"latn"}.number(), 215);

    ASSERT_EQ(hi::iso_15924{"Yiii"}.number(), 460);
    ASSERT_EQ(hi::iso_15924{"YIII"}.number(), 460);
    ASSERT_EQ(hi::iso_15924{"yiii"}.number(), 460);

    ASSERT_THROW(hi::iso_15924{"yi  "}, hi::parse_error);
    ASSERT_THROW(hi::iso_15924{"Foob"}, hi::parse_error);
}

TEST(iso_15925, to_code4)
{
    ASSERT_EQ(hi::iso_15924{215}.code4(), "Latn");
    ASSERT_EQ(hi::iso_15924{460}.code4(), "Yiii");
}

TEST(iso_15925, to_code4_open_type)
{
    ASSERT_EQ(hi::iso_15924{215}.code4_open_type(), "latn");
    ASSERT_EQ(hi::iso_15924{460}.code4_open_type(), "yi  ");
}
