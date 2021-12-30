// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/text/text_fold.hpp"
#include "ttauri/cast.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <tuple>

static std::pair<tt::unicode_general_category, float> text_fold_discriminator(char x) noexcept
{
    switch (x) {
    case 'i': return {tt::unicode_general_category::Ll, 5.0f};
    case 'M': return {tt::unicode_general_category::Lu, 15.0f};
    case ' ': return {tt::unicode_general_category::Zs, 10.0f};
    case '\n': return {tt::unicode_general_category::Zp, 10.0f};
    default:
        return {tt::unicode_general_category::Ll, 10.0f};
    }
}

static std::vector<size_t> test(std::string_view text, float max_line_width) noexcept
{
    auto r = std::vector<size_t>{};

    for (ttlet line_length: tt::text_fold(text.begin(), text.end(), max_line_width, text_fold_discriminator)) {
        r.push_back(line_length);
    }
    return r;
}

template<typename... Args>
static std::vector<size_t> result(Args const &...args) noexcept
{
    return std::vector<size_t>{tt::narrow<size_t>(args)...};
}

TEST(text_fold, single_line)
{
    //                        1
    //              01234567890123456
    ASSERT_EQ(test("hello blue marble", 45.0f), result(6, 5, 6));
    ASSERT_EQ(test("hello blue marble", 55.0f), result(6, 5, 6));
    ASSERT_EQ(test("hello blue marble", 65.0f), result(6, 5, 6));
    ASSERT_EQ(test("hello blue marble", 95.0f), result(6, 5, 6));
    ASSERT_EQ(test("hello blue marble", 105.0f), result(11, 6));
    ASSERT_EQ(test("hello blue marble", 115.0f), result(11, 6));
    ASSERT_EQ(test("hello blue marble", 165.0f), result(11, 6));
    ASSERT_EQ(test("hello blue marble", 175.0f), result(17));
}

TEST(text_fold, single_line_end_space)
{
    //                        1
    //              012345678901234567
    ASSERT_EQ(test("hello blue marble ", 45.0f), result(6, 5, 7));
    ASSERT_EQ(test("hello blue marble ", 55.0f), result(6, 5, 7));
    ASSERT_EQ(test("hello blue marble ", 65.0f), result(6, 5, 7));
    ASSERT_EQ(test("hello blue marble ", 95.0f), result(6, 5, 7));
    ASSERT_EQ(test("hello blue marble ", 105.0f), result(11, 7));
    ASSERT_EQ(test("hello blue marble ", 115.0f), result(11, 7));
    ASSERT_EQ(test("hello blue marble ", 165.0f), result(11, 7));
    ASSERT_EQ(test("hello blue marble ", 175.0f), result(18));
}

TEST(text_fold, single_line_extra_space1)
{
    //                        1
    //              012345678901234567
    ASSERT_EQ(test("hello  blue marble", 45.0f), result(7, 5, 6));
    ASSERT_EQ(test("hello  blue marble", 55.0f), result(7, 5, 6));
    ASSERT_EQ(test("hello  blue marble", 65.0f), result(7, 5, 6));
    ASSERT_EQ(test("hello  blue marble", 75.0f), result(7, 5, 6));
    ASSERT_EQ(test("hello  blue marble", 105.0f), result(7, 5, 6));
    ASSERT_EQ(test("hello  blue marble", 115.0f), result(12, 6));
    ASSERT_EQ(test("hello  blue marble", 125.0f), result(12, 6));
    ASSERT_EQ(test("hello  blue marble", 175.0f), result(12, 6));
    ASSERT_EQ(test("hello  blue marble", 185.0f), result(18));
}

TEST(text_fold, single_line_extra_space2)
{
    //                        1
    //              012345678901234567
    ASSERT_EQ(test("hello blue  marble", 45.0f), result(6, 6, 6));
    ASSERT_EQ(test("hello blue  marble", 55.0f), result(6, 6, 6));
    ASSERT_EQ(test("hello blue  marble", 65.0f), result(6, 6, 6));
    ASSERT_EQ(test("hello blue  marble", 95.0f), result(6, 6, 6));
    ASSERT_EQ(test("hello blue  marble", 105.0f), result(12, 6));
    ASSERT_EQ(test("hello blue  marble", 115.0f), result(12, 6));
    ASSERT_EQ(test("hello blue  marble", 125.0f), result(12, 6));
    ASSERT_EQ(test("hello blue  marble", 175.0f), result(12, 6));
    ASSERT_EQ(test("hello blue  marble", 185.0f), result(18));
}

TEST(text_fold, single_line_paragraph)
{
    //                        1
    //              01234567890123456 7
    ASSERT_EQ(test("hello blue marble\n", 45.0f), result(6, 5, 7));
    ASSERT_EQ(test("hello blue marble\n", 55.0f), result(6, 5, 7));
    ASSERT_EQ(test("hello blue marble\n", 65.0f), result(6, 5, 7));
    ASSERT_EQ(test("hello blue marble\n", 95.0f), result(6, 5, 7));
    ASSERT_EQ(test("hello blue marble\n", 105.0f), result(11, 7));
    ASSERT_EQ(test("hello blue marble\n", 115.0f), result(11, 7));
    ASSERT_EQ(test("hello blue marble\n", 165.0f), result(11, 7));
    ASSERT_EQ(test("hello blue marble\n", 175.0f), result(18));
    ASSERT_EQ(test("hello blue marble\n", 185.0f), result(18));
}
