// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_text_segmentation.hpp"
#include "../file/file_view.hpp"
#include "../charconv.hpp"
#include "../ranges.hpp"
#include "../strings.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <string_view>
#include <span>
#include <format>

using namespace std;
using namespace hi;

struct graphemeBreakTest {
    std::u32string code_points;
    std::vector<bool> break_opportunities;
    std::string comment;
    int lineNr;
};

std::optional<graphemeBreakTest> parsegraphemeBreakTests_line(std::string_view line, int lineNr)
{
    graphemeBreakTest r;

    hilet split_line = split(line, "\t#");
    if (split_line.size() < 2) {
        return {};
    }
    r.comment = std::format("{}: {}", lineNr, split_line[1]);
    r.lineNr = lineNr;

    hilet columns = split(split_line[0]);
    if (columns.size() < 2) {
        return {};
    }

    for (hilet column : columns) {
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
    hilet view = file_view{"graphemeBreakTest.txt"};
    hilet test_data = as_string_view(view);

    std::vector<graphemeBreakTest> r;
    int lineNr = 1;
    for (hilet line : split(test_data, '\n')) {
        if (hilet optionalTest = parsegraphemeBreakTests_line(line, lineNr)) {
            r.push_back(*optionalTest);
        }
        lineNr++;
    }
    return r;
}

TEST(unicode_text_segmentation, breaks_grapheme)
{
    auto tests = parsegraphemeBreakTests();

    for (hilet &test : tests) {
        ASSERT_EQ(test.code_points.size() + 1, test.break_opportunities.size());

        auto state = grapheme_break_state{};

        for (std::size_t i = 0; i < test.code_points.size(); i++) {
            hilet code_point = test.code_points[i];
            hilet break_opportunities = test.break_opportunities[i];

            ASSERT_EQ(breaks_grapheme(code_point, state), break_opportunities) << test.comment;
        }
    }
}
