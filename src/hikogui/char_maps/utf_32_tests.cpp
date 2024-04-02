// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "utf_32.hpp"
#include "random_char.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(char_maps_utf_32_suite) {

TEST_CASE(identity_move)
{
    auto identity_tst = std::u32string{};
    for (size_t i = 0; i != 100; ++i) {
        identity_tst += hi::char_cast<char32_t>(hi::random_char());
    }

    for (size_t i = 0; i != identity_tst.size(); ++i) {
        for (size_t j = i; j != identity_tst.size(); ++j) {
            auto original = identity_tst.substr(i, j - i);

            auto test = original;
            auto *test_ptr = test.data();
            auto result = hi::char_converter<"utf-32", "utf-32">{}(std::move(test));
            auto *result_ptr = result.data();

            // Check for short-string-optimization.
            if (original.size() > sizeof(std::string)) {
                // Check if the string was properly moved.
                REQUIRE(static_cast<void *>(test_ptr) == static_cast<void *>(result_ptr), std::format("i = {}, j = {}", i, j));
            } else {
                REQUIRE(original == result);
            }
        }
    }
}

TEST_CASE(identity_copy)
{
    auto identity_tst = std::u32string{};
    for (size_t i = 0; i != 100; ++i) {
        identity_tst += hi::char_cast<char32_t>(hi::random_char());
    }

    for (size_t i = 0; i != identity_tst.size(); ++i) {
        for (size_t j = i; j != identity_tst.size(); ++j) {
            auto test = identity_tst.substr(i, j - i);

            auto result = hi::char_converter<"utf-32", "utf-32">{}(test);

            // Check for short-string-optimization.
            REQUIRE(test == result);
        }
    }
}

TEST_CASE(identity_invalid_chars)
{
    auto const invalid_tst = std::u32string{U"abcdefghijklmnopqrstuvwxy\xd800zABCDEFGHIJKLMNOPQRSTUVWXY\x00110000Z0123456789"};
    auto const invalid_exp = std::u32string{U"abcdefghijklmnopqrstuvwxy\ufffdzABCDEFGHIJKLMNOPQRSTUVWXY\ufffdZ0123456789"};

    for (size_t i = 0; i != invalid_tst.size(); ++i) {
        for (size_t j = i; j != invalid_tst.size(); ++j) {
            auto test = invalid_tst.substr(i, j - i);
            auto expected = invalid_exp.substr(i, j - i);

            auto result = hi::char_converter<"utf-32", "utf-32">{}(test);

            // Check for short-string-optimization.
            REQUIRE(expected == result);
        }
    }
}

};
