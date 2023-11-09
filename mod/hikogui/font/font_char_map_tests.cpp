// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "font_char_map.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>

TEST(font_char_map, add_and_search)
{
    auto cm = hi::font_char_map{};

    cm.add(U'a', U'z', 100);
    cm.add(U'5', U'6', 305);
    cm.add(U'0', U'3', 200);
    cm.add(U'4', U'4', 204);
    // '7' is missing.
    cm.add(U'8', U'9', 208);

    cm.prepare();

    ASSERT_EQ(cm.find(U'a'), 100);
    ASSERT_EQ(cm.find(U'b'), 101);
    ASSERT_EQ(cm.find(U'z'), 125);
    ASSERT_EQ(cm.find(U'0'), 200);
    ASSERT_EQ(cm.find(U'1'), 201);
    ASSERT_EQ(cm.find(U'2'), 202);
    ASSERT_EQ(cm.find(U'3'), 203);
    ASSERT_EQ(cm.find(U'4'), 204);
    ASSERT_EQ(cm.find(U'5'), 305);
    ASSERT_EQ(cm.find(U'6'), 306);
    ASSERT_EQ(cm.find(U'7'), 0xffff);
    ASSERT_EQ(cm.find(U'8'), 208);
    ASSERT_EQ(cm.find(U'9'), 209);
}
