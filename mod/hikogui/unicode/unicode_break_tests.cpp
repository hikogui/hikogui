// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_word_break.hpp"
#include "unicode_sentence_break.hpp"
#include "unicode_line_break.hpp"
#include "unicode_grapheme_cluster_break.hpp"
#include "unicode_description.hpp"
#include "../file/file.hpp"
#include "../path/path.hpp"
#include "../algorithm/algorithm.hpp"
#include "../coroutine/coroutine.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <span>
#include <format>
#include <ranges>



namespace {

struct test_type {
    std::u32string code_points;
    std::vector<hi::unicode_break_opportunity> expected;
    std::string comment;
    int line_nr;
};

static std::optional<test_type> parse_test_line(std::string_view line, int line_nr)
{
    auto r = test_type{};

    hilet split_line = hi::split(line, "\t#");
    if (split_line.size() < 2) {
        return {};
    }
    r.comment = std::format("{}: {}", line_nr, split_line[1]);
    r.line_nr = line_nr;

    hilet columns = hi::split(split_line[0]);
    if (columns.size() < 2) {
        return {};
    }

    for (hilet column : columns) {
        if (column == "") {
            // Empty.
        } else if (column == "\xc3\xb7") {
            r.expected.push_back(hi::unicode_break_opportunity::yes);
        } else if (column == "\xc3\x97") {
            r.expected.push_back(hi::unicode_break_opportunity::no);
        } else {
            auto const code_point = std::stoi(std::string(column), nullptr, 16);
            r.code_points += hi::char_cast<char32_t>(code_point);
        }
    }

    return {std::move(r)};
}

static hi::generator<test_type> parse_tests(std::filesystem::path filename)
{
    hilet view = hi::file_view(filename);
    hilet test_data = as_string_view(view);

    int line_nr = 1;
    for (hilet line_view : std::views::split(test_data, std::string_view{"\n"})) {
        hilet line = std::string_view{line_view.begin(), line_view.end()};
        if (hilet optional_test = parse_test_line(line, line_nr)) {
            co_yield *optional_test;
        }
        line_nr++;
    }
}

} // namespace

TEST(unicode_break, grapheme_break)
{
    for (hilet& test : parse_tests(hi::library_source_dir() / "tests" / "data" / "GraphemeBreakTest.txt")) {
        hilet result = hi::unicode_grapheme_break(test.code_points.begin(), test.code_points.end());

        ASSERT_EQ(test.expected, result) << test.comment;
    }
}

TEST(unicode_break, word_break)
{
    for (hilet& test : parse_tests(hi::library_source_dir() / "tests" / "data" / "WordBreakTest.txt")) {
        hilet result =
            hi::unicode_word_break(test.code_points.begin(), test.code_points.end(), [](hilet code_point) -> decltype(auto) {
                return code_point;
            });

        ASSERT_EQ(test.expected, result) << test.comment;
    }
}

TEST(unicode_break, sentence_break)
{
    for (hilet& test : parse_tests(hi::library_source_dir() / "tests" / "data" / "SentenceBreakTest.txt")) {
        hilet result =
            hi::unicode_sentence_break(test.code_points.begin(), test.code_points.end(), [](hilet code_point) -> decltype(auto) {
                return code_point;
            });

        ASSERT_EQ(test.expected, result) << test.comment;
    }
}

TEST(unicode_break, line_break)
{
    for (hilet& test : parse_tests(hi::library_source_dir() / "tests" / "data" / "LineBreakTest.txt")) {
        auto result =
            hi::unicode_line_break(test.code_points.begin(), test.code_points.end(), [](hilet code_point) -> decltype(auto) {
                return code_point;
            });

        // The algorithm produces mandatory-break in the result, but LineBreakTest.txt only has break/no-break.
        for (auto& x : result) {
            if (x == hi::unicode_break_opportunity::mandatory) {
                x = hi::unicode_break_opportunity::yes;
            }
        }

        ASSERT_EQ(test.expected, result) << test.comment;
    }
}
