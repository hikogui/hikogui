// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "utf_8.hpp"
#include "random_char.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <format>

using namespace std;
using namespace hi;

static void push_utf_8(char32_t code_point, std::u8string& str) noexcept
{
    if (code_point < 0x80) {
        str += char_cast<char8_t>(code_point);
    } else if (code_point < 0x800) {
        str += char_cast<char8_t>(0xc0 + (code_point >> 6));
        str += char_cast<char8_t>(0x80 + (code_point & 0x3f));
    } else if (code_point < 0x01'0000) {
        str += char_cast<char8_t>(0xe0 + (code_point >> 12));
        str += char_cast<char8_t>(0x80 + ((code_point >> 6) & 0x3f));
        str += char_cast<char8_t>(0x80 + (code_point & 0x3f));
    } else {
        str += char_cast<char8_t>(0xf0 + (code_point >> 18));
        str += char_cast<char8_t>(0x80 + ((code_point >> 12) & 0x3f));
        str += char_cast<char8_t>(0x80 + ((code_point >> 6) & 0x3f));
        str += char_cast<char8_t>(0x80 + (code_point & 0x3f));
    }
}

static bool valid_split(std::u8string_view str)
{
    if (str.size() == 0) {
        return true;
    } else if (((str.front() & 0xc0) == 0x80)) {
        return false;
    } else if ((str.back() & 0x80)) {
        return false;
    } else {
        return true;
    }
}

TEST(char_maps_utf_8, identity_move)
{
    auto identity_tst = std::u8string{};
    for (auto i = 0_uz; i != 100; ++i) {
        push_utf_8(random_char(), identity_tst);
    }

    for (auto i = 0_uz; i != identity_tst.size(); ++i) {
        for (auto j = i; j != identity_tst.size(); ++j) {
            auto original = identity_tst.substr(i, j - i);
            if (not valid_split(original)) {
                continue;
            }

            auto test = original;
            auto *test_ptr = test.data();
            auto result = char_converter<"utf-8", "utf-8">{}.convert<std::u8string>(std::move(test));
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

TEST(char_maps_utf_8, identity_copy)
{
    auto identity_tst = std::u8string{};
    for (auto i = 0_uz; i != 100; ++i) {
        push_utf_8(random_char(), identity_tst);
    }

    for (auto i = 0_uz; i != identity_tst.size(); ++i) {
        for (auto j = i; j != identity_tst.size(); ++j) {
            auto test = identity_tst.substr(i, j - i);
            if (not valid_split(test)) {
                continue;
            }

            auto result = char_converter<"utf-8", "utf-8">{}.convert<std::u8string>(test);

            // Check for short-string-optimization.
            ASSERT_EQ(test, result);
        }
    }
}

TEST(char_maps_utf_8, identity_invalid_chars)
{
    //                                   ascii                 invalid overlong    surrogate       short   short
    hilet invalid_tst_ = std::string{"abcdefghijklmnopqrstuvwxy\xfezAG\xe0\x80\x80H\xed\xa0\xadIJK\xe0LMNO\xe0\x80P"};
    //                                                            fallback repl   repl     fallback  repl
    hilet invalid_exp = std::u8string{u8"abcdefghijklmnopqrstuvwxy\u00fezAG\ufffdH\ufffdIJK\u00e0LMNO\ufffdP"};

    // MSVC bug: https://developercommunity.visualstudio.com/t/escape-sequences-in-unicode-string-literals-are-ov/260684
    // hex-escape are treated as code-points (wrong) instead of code-units (correct)
    auto invalid_tst = std::u8string{};
    for (auto i = 0_uz; i != invalid_tst_.size(); ++i) {
        invalid_tst += char_cast<char8_t>(invalid_tst_[i]);
    }

    for (auto i = 0_uz; i != invalid_tst.size(); ++i) {
        auto test = invalid_tst.substr(0, i);
        if (not valid_split(test)) {
            continue;
        }

        auto result = char_converter<"utf-8", "utf-8">{}.convert<std::u8string>(test);
        auto expected = invalid_exp.substr(0, result.size());

        // Check for short-string-optimization.
        ASSERT_EQ(expected, result);
    }
}
