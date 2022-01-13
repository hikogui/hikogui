// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/codec/UTF.hpp"
#include "ttauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>

TEST(UTF, utf8_to_utf32)
{
    ASSERT_EQ(tt::to_u32string("\u4e16"), std::u32string(U"\u4e16"));
    ASSERT_EQ(tt::to_u32string("Hello \u4e16\u754c"), std::u32string(U"Hello \u4e16\u754c"));
}
