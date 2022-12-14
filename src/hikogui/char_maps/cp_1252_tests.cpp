// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "cp_1252.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <format>

using namespace std;
using namespace hi;

TEST(char_maps_cp_1252, identity_move)
{
    auto identity_tst = std::string{};
    for (auto i = 0_uz; i != 256; ++i) {
        identity_tst += char_cast<char>(i);
    }

    for (auto i = 0_uz; i != identity_tst.size(); ++i) {
        for (auto j = i; j != identity_tst.size(); ++j) {
            auto original = identity_tst.substr(i, j - i);

            auto test = original;
            auto *test_ptr = test.data();
            auto result = char_converter<"cp-1252", "cp-1252">{}(std::move(test));
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

TEST(char_maps_cp_1252, identity_copy)
{
    auto identity_tst = std::string{};
    for (auto i = 0_uz; i != 256; ++i) {
        identity_tst += char_cast<char>(i);
    }

    for (auto i = 0_uz; i != identity_tst.size(); ++i) {
        for (auto j = i; j != identity_tst.size(); ++j) {
            auto test = identity_tst.substr(i, j - i);

            auto result = char_converter<"cp-1252", "cp-1252">{}(test);

            // Check for short-string-optimization.
            ASSERT_EQ(test, result);
        }
    }
}

