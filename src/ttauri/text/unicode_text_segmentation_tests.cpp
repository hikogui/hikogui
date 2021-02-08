// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/text/unicode_text_segmentation.hpp"
#include "ttauri/file_view.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <span>
#include <fmt/format.h>

using namespace std;
using namespace tt;

struct graphemeBreakTest {
    std::u32string codePoints;
    std::vector<bool> breakOpertunities;
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
    r.comment = fmt::format("{}: {}", lineNr, split_line[1]);
    r.lineNr = lineNr;

    ttlet columns = split(split_line[0]);
    if (columns.size() < 2) {
        return {};
    }

    for (ttlet column : columns) {
        if (column == "") {
            // Empty.
        } else if (column == "\xc3\xb7") {
            r.breakOpertunities.push_back(true);
        } else if (column == "\xc3\x97") {
            r.breakOpertunities.push_back(false);
        } else {
            auto codePoint = static_cast<char32_t>(std::stoi(std::string(column), nullptr, 16));
            r.codePoints += codePoint;
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
        ASSERT_EQ(test.codePoints.size() + 1, test.breakOpertunities.size());

        auto state = grapheme_break_state{};

        for (size_t i = 0; i < test.codePoints.size(); i++) {
            ttlet codePoint = test.codePoints[i];
            ttlet breakOpertunity = test.breakOpertunities[i];

            ASSERT_EQ(breaks_grapheme(codePoint, state), breakOpertunity) << test.comment;
        }
    }
}
