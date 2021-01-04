// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/text/unicode_bidi.cpp"
#include "ttauri/FileView.hpp"
#include "ttauri/charconv.hpp"
#include "ttauri/ranges.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <span>
#include <fmt/format.h>

using namespace tt;
using namespace tt::detail;
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

    [[nodiscard]] std::vector<detail::unicode_bidi_char_info> get_input() const noexcept
    {
        auto r = std::vector<detail::unicode_bidi_char_info>{};
        auto index = 0;
        for (auto cls : input) {
            r.emplace_back(index++, cls);
        }
        return r;
    }

    [[nodiscard]] std::vector<unicode_bidi_class> get_paragraph_directions() const noexcept
    {
        auto r = std::vector<unicode_bidi_class>{};

        if (test_for_LTR) {
            r.push_back(unicode_bidi_class::L);
        }
        if (test_for_RTL) {
            r.push_back(unicode_bidi_class::R);
        }
        if (test_for_auto) {
            r.push_back(unicode_bidi_class::unknown);
        }

        return r;
    }
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

generator<unicode_bidi_test> parse_bidi_test(int test_line_nr = -1)
{
    ttlet view = FileView(URL("file:BidiTest.txt"));
    ttlet test_data = view.string_view();

    auto levels = std::vector<int>{};
    auto reorder = std::vector<int>{};

    int line_nr = 1;
    for (ttlet line : tt::views::split(test_data, "\n")) {
        if (line.empty() || line.starts_with("#")) {
            // Comment and empty lines.
        } else if (line.starts_with("@Levels:")) {
            levels = parse_bidi_test_levels(line.substr(8));
        } else if (line.starts_with("@Reorder:")) {
            reorder = parse_bidi_test_reorder(line.substr(9));
        } else {
            auto data = parse_bidi_test_data_line(line, levels, reorder, line_nr);
            if (test_line_nr == -1 || line_nr == test_line_nr) {
                co_yield data;
            }
        }

        if (line_nr == test_line_nr) {
            break;
        }

        line_nr++;
    }
}

TEST(unicode_bidi, first)
{
    for (auto test : parse_bidi_test()) {
        for (auto paragraph_direction : test.get_paragraph_directions()) {
            auto test_parameters = unicode_bidi_test_parameters{};
            test_parameters.enable_mirrored_brackets = false;
            test_parameters.enable_line_separator = false;
            test_parameters.force_paragraph_direction = paragraph_direction;

            auto input = test.get_input();
            auto first = std::begin(input);
            auto last = std::end(input);

            last = unicode_bidi_P1(first, last, test_parameters);

            // We are using the index from the iterator to find embedded levels
            // in input-order. We ignore all elements that where removed by X9.
            for (auto it = first; it != last; ++it) {
                ttlet expected_embedding_level = test.levels[it->index];

                ASSERT_TRUE(expected_embedding_level == -1 || expected_embedding_level == it->embedding_level);
            }

            ASSERT_EQ(std::distance(first, last), std::ssize(test.reorder));

            auto index = 0;
            for (auto it = first; it != last; ++it, ++index) {
                ttlet expected_input_index = test.reorder[index];

                ASSERT_TRUE(expected_input_index == -1 || expected_input_index == it->index);
            }
        }
    }
}
