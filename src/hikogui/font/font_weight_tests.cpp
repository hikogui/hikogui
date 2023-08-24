// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "font_weight.hpp"
#include <gtest/gtest.h>

TEST(font_weight, regular_alternatives)
{
    auto range = hi::alternatives(hi::font_weight::regular);
    auto it = range.begin();
    ASSERT_NE(it, range.end());
    ASSERT_EQ(*it++, hi::font_weight::regular);
    ASSERT_EQ(*it++, hi::font_weight::light);
    ASSERT_EQ(*it++, hi::font_weight::medium);
    ASSERT_EQ(*it++, hi::font_weight::extra_light);
    ASSERT_EQ(*it++, hi::font_weight::semi_bold);
    ASSERT_EQ(*it++, hi::font_weight::thin);
    ASSERT_EQ(*it++, hi::font_weight::bold);
    ASSERT_EQ(*it++, hi::font_weight::extra_bold);
    ASSERT_EQ(*it++, hi::font_weight::black);
    ASSERT_EQ(*it++, hi::font_weight::extra_black);
    ASSERT_EQ(it, range.end());
}

TEST(font_weight, bold_alternatives)
{
    auto range = hi::alternatives(hi::font_weight::bold);
    auto it = range.begin();
    ASSERT_NE(it, range.end());
    ASSERT_EQ(*it++, hi::font_weight::bold);
    ASSERT_EQ(*it++, hi::font_weight::semi_bold);
    ASSERT_EQ(*it++, hi::font_weight::extra_bold);
    ASSERT_EQ(*it++, hi::font_weight::medium);
    ASSERT_EQ(*it++, hi::font_weight::black);
    ASSERT_EQ(*it++, hi::font_weight::regular);
    ASSERT_EQ(*it++, hi::font_weight::extra_black);
    ASSERT_EQ(*it++, hi::font_weight::light);
    ASSERT_EQ(*it++, hi::font_weight::extra_light);
    ASSERT_EQ(*it++, hi::font_weight::thin);
    ASSERT_EQ(it, range.end());
}
