// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/unicode/unicode_text_segmentation.hpp"
#include "ttauri/file_view.hpp"
#include "ttauri/charconv.hpp"
#include "ttauri/ranges.hpp"
#include "ttauri/strings.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <string_view>
#include <span>
#include <format>

using namespace std;
using namespace tt;

struct graphemeBreakTest {
    std::u32string code_points;
    std::vector<bool> break_opportunities;
    std::string comment;
    int lineNr;
};

std::optional<graphemeBreakTest> parsegraphemeBreakTests_line(std::string_view line, int lineNr)
{
    graphemeBreakTest r;

    ttlet split_line = split(line, "\t#");
    if (split_line.size() < 2) {
        return {};
    }
    r.comment = std::format("{}: {}", lineNr, split_line[1]);
    r.lineNr = lineNr;

    ttlet columns = split(split_line[0]);
    if (columns.size() < 2) {
        return {};
    }

    for (ttlet column : columns) {
        if (column == "") {
            // Empty.
        } else if (column == "\xc3\xb7") {
            r.break_opportunities.push_back(true);
        } else if (column == "\xc3\x97") {
            r.break_opportunities.push_back(false);
        } else {
            auto code_point = static_cast<char32_t>(std::stoi(std::string(column), nullptr, 16));
            r.code_points += code_point;
        }
    }

    return r;
}

std::vector<graphemeBreakTest> parsegraphemeBreakTests()
{
    ttlet view = file_view(URL("file:graphemeBreakTest.txt"));
    ttlet test_data = view.string_view();

    std::vector<graphemeBreakTest> r;
    int lineNr = 1;
    for (ttlet line : split(test_data, '\n')) {
        if (ttlet optionalTest = parsegraphemeBreakTests_line(line, lineNr)) {
            r.push_back(*optionalTest);
        }
        lineNr++;
    }
    return r;
}

TEST(unicode_text_segmentation, breaks_grapheme)
{
    auto tests = parsegraphemeBreakTests();

    for (ttlet &test : tests) {
        ASSERT_EQ(test.code_points.size() + 1, test.break_opportunities.size());

        auto state = grapheme_break_state{};

        for (std::size_t i = 0; i < test.code_points.size(); i++) {
            ttlet code_point = test.code_points[i];
            ttlet break_opportunities = test.break_opportunities[i];

            ASSERT_EQ(breaks_grapheme(code_point, state), break_opportunities) << test.comment;
        }
    }
}
