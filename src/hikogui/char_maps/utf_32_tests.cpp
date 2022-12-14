// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "utf_32.hpp"
#include "random_char.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <format>
#include <random>

using namespace std;
using namespace hi;


TEST(char_maps_utf_32, identity_move)
{
    auto identity_tst = std::u32string{};
    for (auto i = 0_uz; i != 100; ++i) {
        identity_tst += char_cast<char32_t>(random_char());
    }

    for (auto i = 0_uz; i != identity_tst.size(); ++i) {
        for (auto j = i; j != identity_tst.size(); ++j) {
            auto original = identity_tst.substr(i, j - i);

            auto test = original;
            auto *test_ptr = test.data();
            auto result = char_converter<"utf-32", "utf-32">{}(std::move(test));
            auto *result_ptr = result.data();

            // Check for short-string-optimization.
            if (original.size() > sizeof(std::string)) {
                // Check if the string was properly moved.
                ASSERT_EQ(test_ptr, result_ptr) << i << j;
            } else {
                ASSERT_EQ(original, result);
            }
        }
    }
}

TEST(char_maps_utf_32, identity_copy)
{
    auto identity_tst = std::u32string{};
    for (auto i = 0_uz; i != 100; ++i) {
        identity_tst += char_cast<char32_t>(random_char());
    }

    for (auto i = 0_uz; i != identity_tst.size(); ++i) {
        for (auto j = i; j != identity_tst.size(); ++j) {
            auto test = identity_tst.substr(i, j - i);

            auto result = char_converter<"utf-32", "utf-32">{}(test);

            // Check for short-string-optimization.
            ASSERT_EQ(test, result);
        }
    }
}

TEST(char_maps_utf_32, identity_invalid_chars)
{
    hilet invalid_tst = std::u32string{U"abcdefghijklmnopqrstuvwxy\xd800zABCDEFGHIJKLMNOPQRSTUVWXY\x00110000Z0123456789"};
    hilet invalid_exp = std::u32string{U"abcdefghijklmnopqrstuvwxy\ufffdzABCDEFGHIJKLMNOPQRSTUVWXY\ufffdZ0123456789"};

    for (auto i = 0_uz; i != invalid_tst.size(); ++i) {
        for (auto j = i; j != invalid_tst.size(); ++j) {
            auto test = invalid_tst.substr(i, j - i);
            auto expected = invalid_exp.substr(i, j - i);

            auto result = char_converter<"utf-32", "utf-32">{}(test);

            // Check for short-string-optimization.
            ASSERT_EQ(expected, result);
        }
    }
}
