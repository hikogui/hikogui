// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ascii.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <format>

using namespace std;
using namespace hi;

static auto invalid_tst = std::string{"abcdefghijklmnopqrstuvwxy\x80zABCDEFGHIJKLMNOPQRSTUVWXY\xffZ0123456789"};
static auto invalid_exp = std::string{"abcdefghijklmnopqrstuvwxy?zABCDEFGHIJKLMNOPQRSTUVWXY?Z0123456789"};

TEST(char_maps_ascii, identity_move)
{
    auto identity_tst = std::string{};
    for (auto i = 0_uz; i != 128; ++i) {
        identity_tst += char_cast<char>(i);
    }

    for (auto i = 0_uz; i != identity_tst.size(); ++i) {
        for (auto j = i; j != identity_tst.size(); ++j) {
            auto original = identity_tst.substr(i, j - i);

            auto test = original;
            auto *test_ptr = test.data();
            auto result = char_converter<"ascii", "ascii">{}(std::move(test));
            auto *result_ptr = result.data();

            // Check for short-string-optimization.
            if (original.size() > sizeof(std::string)) {
                // Check if the string was properly moved.
                ASSERT_EQ(test_ptr, result_ptr) << std::format("original = '{}', result = '{}'", original, result);
            } else {
                ASSERT_EQ(original, result);
            }
        }
    }
}

TEST(char_maps_ascii, identity_copy)
{
    auto identity_tst = std::string{};
    for (auto i = 0_uz; i != 128; ++i) {
        identity_tst += char_cast<char>(i);
    }

    for (auto i = 0_uz; i != identity_tst.size(); ++i) {
        for (auto j = i; j != identity_tst.size(); ++j) {
            auto test = identity_tst.substr(i, j - i);

            auto result = char_converter<"ascii", "ascii">{}(test);

            // Check for short-string-optimization.
            ASSERT_EQ(test, result);
        }
    }
}

TEST(char_maps_ascii, identity_invalid_chars)
{
    for (auto i = 0_uz; i != invalid_tst.size(); ++i) {
        for (auto j = i; j != invalid_tst.size(); ++j) {
            auto test = invalid_tst.substr(i, j - i);
            auto expected = invalid_exp.substr(i, j - i);

            auto result = char_converter<"ascii", "ascii">{}(test);

            // Check for short-string-optimization.
            ASSERT_EQ(expected, result);
        }
    }
}
