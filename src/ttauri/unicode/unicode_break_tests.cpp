// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/unicode/unicode_word_break.hpp"
#include "ttauri/unicode/unicode_sentence_break.hpp"
#include "ttauri/unicode/unicode_line_break.hpp"
#include "ttauri/unicode/unicode_description.hpp"
#include "ttauri/file_view.hpp"
#include "ttauri/strings.hpp"
#include "ttauri/coroutine.hpp"
#include "ttauri/ranges.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <span>
#include <format>

namespace {

struct test_type {
    std::u32string code_points;
    std::vector<tt::unicode_break_opportunity> expected;
    std::string comment;
    int line_nr;
};

static std::optional<test_type> parse_test_line(std::string_view line, int line_nr)
{
    auto r = test_type{};

    ttlet split_line = tt::split(line, "\t#");
    if (split_line.size() < 2) {
        return {};
    }
    r.comment = std::format("{}: {}", line_nr, split_line[1]);
    r.line_nr = line_nr;

    ttlet columns = tt::split(split_line[0]);
    if (columns.size() < 2) {
        return {};
    }

    for (ttlet column : columns) {
        if (column == "") {
            // Empty.
        } else if (column == "\xc3\xb7") {
            r.expected.push_back(tt::unicode_break_opportunity::yes);
        } else if (column == "\xc3\x97") {
            r.expected.push_back(tt::unicode_break_opportunity::no);
        } else {
            auto code_point = static_cast<char32_t>(std::stoi(std::string(column), nullptr, 16));
            r.code_points += code_point;
        }
    }

    return {std::move(r)};
}

static tt::generator<test_type> parse_tests(std::string_view filename)
{
    ttlet view = tt::file_view(tt::URL(filename));
    ttlet test_data = view.string_view();

    int line_nr = 1;
    for (ttlet line : tt::views::split(test_data, "\n")) {
        if (ttlet optional_test = parse_test_line(line, line_nr)) {
            co_yield *optional_test;
        }
        line_nr++;
    }
}

} // namespace

TEST(unicode_break, word_break)
{
    for (ttlet &test : parse_tests("WordBreakTest.txt")) {
        ttlet result =
            tt::unicode_word_break(test.code_points.begin(), test.code_points.end(), [](ttlet code_point) -> decltype(auto) {
                return tt::unicode_description::find(code_point);
            });

        ASSERT_EQ(test.expected, result) << test.comment;
    }
}

TEST(unicode_break, sentence_break)
{
    for (ttlet &test : parse_tests("SentenceBreakTest.txt")) {
        ttlet result =
            tt::unicode_sentence_break(test.code_points.begin(), test.code_points.end(), [](ttlet code_point) -> decltype(auto) {
                return tt::unicode_description::find(code_point);
            });

        ASSERT_EQ(test.expected, result) << test.comment;
    }
}

TEST(unicode_break, line_break)
{
    for (ttlet &test : parse_tests("LineBreakTest.txt")) {
        auto result =
            tt::unicode_line_break(test.code_points.begin(), test.code_points.end(), [](ttlet code_point) -> decltype(auto) {
                return tt::unicode_description::find(code_point);
            });

        // The algorithm produces mandatory-break in the result, but LineBreakTest.txt only has break/no-break.
        for (auto &x : result) {
            if (x == tt::unicode_break_opportunity::mandatory) {
                x = tt::unicode_break_opportunity::yes;
            }
        }

        ASSERT_EQ(test.expected, result) << test.comment;
    }
}
