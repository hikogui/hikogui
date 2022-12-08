// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "utf_8.hpp"
#include "utf_32.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <format>

using namespace std;
using namespace hi;

TEST(char_converter, utf8_to_utf32_large_char_at_end)
{
    // "seven" in Hebrew.
    auto test = std::string{"\xd7\xa9\xd7\x91\xd7\xa2\xd7\x94"};
    auto expected = std::u32string{U"\u05E9\u05D1\u05E2\u05d4"};

    auto result = char_converter<"utf-8", "utf-32">{}.convert<std::u32string>(test);

    ASSERT_EQ(result, expected);
}
