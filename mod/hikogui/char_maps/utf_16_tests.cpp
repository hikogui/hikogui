// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "utf_16.hpp"
#include "random_char.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <format>
#include <random>



using namespace std;
using namespace hi;

static void push_utf_16(char32_t code_point, std::u16string& str) noexcept
{
    if (code_point < 0x01'0000) {
        str += char_cast<char16_t>(code_point);
    } else {
        code_point -= 0x01'0000;
        str += char_cast<char16_t>(0xd800 + (code_point >> 10));
        str += char_cast<char16_t>(0xdc00 + (code_point & 0x03ff));
    }
}

static bool valid_split(std::u16string_view str)
{
    if (str.size() == 0) {
        return true;
    } else if (str.size() == 1) {
        return str.front() < 0xd800 or str.front() >= 0xe000;
    } else {
        if (str.front() >= 0xdc00 and str.front() < 0xe000) {
            return false;
        } else if (str.back() >= 0xd800 and str.back() < 0xdc00) {
            return false;
        } else {
            return true;
        }
    }
}

TEST(char_maps_utf_16, identity_move)
{
    auto identity_tst = std::u16string{};
    for (auto i = 0_uz; i != 100; ++i) {
        push_utf_16(random_char(), identity_tst);
    }

    for (auto i = 0_uz; i != identity_tst.size(); ++i) {
        for (auto j = i; j != identity_tst.size(); ++j) {
            auto original = identity_tst.substr(i, j - i);
            if (not valid_split(original)) {
                continue;
            }

            auto test = original;
            auto *test_ptr = test.data();
            auto result = char_converter<"utf-16", "utf-16">{}(std::move(test));
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

TEST(char_maps_utf_16, identity_copy)
{
    auto identity_tst = std::u16string{};
    for (auto i = 0_uz; i != 100; ++i) {
        push_utf_16(random_char(), identity_tst);
    }

    for (auto i = 0_uz; i != identity_tst.size(); ++i) {
        for (auto j = i; j != identity_tst.size(); ++j) {
            auto test = identity_tst.substr(i, j - i);
            if (not valid_split(test)) {
                continue;
            }

            auto result = char_converter<"utf-16", "utf-16">{}(test);

            // Check for short-string-optimization.
            ASSERT_EQ(test, result);
        }
    }
}


TEST(char_maps_utf_16, identity_invalid_chars)
{
    hilet invalid_tst = std::u16string{
        u"abcdefghijklmnopqrstuvwxy\xd800zA\U00012345BCD\xd800\U00012345E\xdc00\U00012345FGHIJKLMNOPQRSTUVWXY\xdc00Z0123456789"};
    hilet invalid_exp = std::u16string{
        u"abcdefghijklmnopqrstuvwxy\ufffdzA\U00012345BCD\ufffd\U00012345E\ufffd\U00012345FGHIJKLMNOPQRSTUVWXY\ufffdZ0123456789"};

    for (auto i = 0_uz; i != invalid_tst.size(); ++i) {
        for (auto j = i; j != invalid_tst.size(); ++j) {
            auto test = invalid_tst.substr(i, j - i);
            if (not valid_split(test)) {
                continue;
            }

            auto expected = invalid_exp.substr(i, j - i);

            auto result = char_converter<"utf-16", "utf-16">{}(test);

            // Check for short-string-optimization.
            ASSERT_EQ(expected, result);
        }
    }
}
