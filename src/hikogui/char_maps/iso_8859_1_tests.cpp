// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "iso_8859_1.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <format>

using namespace std;
using namespace hi;

static auto normal_tst = std::string{"abcdefghijklmnopqrstuvwxy\x80zABCDEFGHIJKLMNOPQRSTUVWXY\xffZ0123456789"};


TEST(char_maps_iso_8859_1, identity_move)
{
    for (auto i = 0_uz; i != normal_tst.size(); ++i) {
        for (auto j = i; j != normal_tst.size(); ++j) {
            auto original = normal_tst.substr(i, j - i);

            auto test = original;
            auto *test_ptr = test.data();
            auto result = char_converter<"iso-8859-1", "iso-8859-1">{}.convert(std::move(test));
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

TEST(char_maps_iso_8859_1, identity_copy)
{
    for (auto i = 0_uz; i != normal_tst.size(); ++i) {
        for (auto j = i; j != normal_tst.size(); ++j) {
            auto test = normal_tst.substr(i, j - i);

            auto result = char_converter<"iso-8859-1", "iso-8859-1">{}.convert(test);

            // Check for short-string-optimization.
            ASSERT_EQ(test, result);
        }
    }
}

