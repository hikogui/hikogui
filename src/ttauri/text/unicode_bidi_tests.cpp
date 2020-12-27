// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/text/unicode_bidi.hpp"
#include "ttauri/FileView.hpp"
#include "ttauri/charconv.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <span>
#include <fmt/format.h>

using namespace tt;
using namespace std;

struct unicode_bidi_test {
    std::vector<int> levels;
    std::vector<int> reorder;
    int line_nr;

    std::vector<unicode_bidi_class> input;
    bool test_for_LTR = false;
    bool test_for_RTL = false;
    bool test_for_auto = false;

    [[nodiscard]] unicode_bidi_test(std::vector<int> const &levels, std::vector<int> const &reorder, int line_nr) noexcept :
        levels(levels), reorder(reorder), line_nr(line_nr) {}
};

[[nodiscard]] static std::vector<int> parse_bidi_test_levels(std::string_view line) noexcept
{
    auto r = std::vector<int>{};
    for (ttlet value : tt::split(tt::strip(line))) {
        if (value == "x") {
            r.push_back(-1);
        } else {
            r.push_back(tt::from_string<int>(value));
        }
    }
    return r;
}

[[nodiscard]] static std::vector<int> parse_bidi_test_reorder(std::string_view line) noexcept
{
    auto r = std::vector<int>{};
    for (ttlet value : split(strip(line))) {
        if (value == "x") {
            r.push_back(-1);
        } else {
            r.push_back(tt::from_string<int>(value));
        }
    }
    return r;
}

[[nodiscard]] static unicode_bidi_test
parse_bidi_test_data_line(std::string_view line, std::vector<int> const &levels, std::vector<int> const &reorder, int level_nr) noexcept
{
    auto r = unicode_bidi_test{levels, reorder, level_nr};

    auto line_s = split(line, ';');
    
    for (auto bidi_class_str : split(strip(line_s[0]))) {
        r.input.push_back(unicode_bidi_class_from_string(bidi_class_str));
    }

    auto bitset = tt::from_string<int>(strip(line_s[1]), 16);
    r.test_for_auto = (bitset & 1) != 0;
    r.test_for_LTR = (bitset & 2) != 0;
    r.test_for_RTL = (bitset & 4) != 0;

    return r;
}

std::vector<unicode_bidi_test> parse_bidi_test()
{
    ttlet view = FileView(URL("file:BidiTest.txt"));
    ttlet test_data = view.string_view();

    auto levels = std::vector<int>{};
    auto reorder = std::vector<int>{};

    std::vector<unicode_bidi_test> r;
    int line_nr = 1;
    for (ttlet line : split(test_data, "\n")) {
        if (line.empty() || line.starts_with("#")) {
            // Comment and empty lines.
        } else if (line.starts_with("@Levels:")) {
            levels = parse_bidi_test_levels(line.substr(8));
        } else if (line.starts_with("@Reorder:")) {
            reorder = parse_bidi_test_reorder(line.substr(9));
        } else {
            auto data = parse_bidi_test_data_line(line, levels, reorder, line_nr);
            r.push_back(data);
        }

        line_nr++;
    }
    return r;
}

TEST(unicode_bidi, first)
{

}
