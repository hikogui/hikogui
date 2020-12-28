// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/text/unicode_bidi.cpp"
#include "ttauri/FileView.hpp"
#include "ttauri/charconv.hpp"
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

    [[nodiscard]] std::vector<int> get_paragraph_embedding_levels() const noexcept
    {
        auto r = std::vector<int>{};

        if (test_for_LTR) {
            r.push_back(0);
        }
        if (test_for_RTL) {
            r.push_back(1);
        }
        if (test_for_auto) {
            r.push_back(-1);
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
        if (line_nr == 128) {
            break;
        }
    }
    return r;
}

TEST(unicode_bidi, first)
{
    auto tests = parse_bidi_test();

    for (auto test : tests) {
        for (auto paragraph_embedding_level : test.get_paragraph_embedding_levels()) {
            auto input = test.get_input();
            auto first = std::begin(input);
            auto last = std::end(input);

            if (paragraph_embedding_level == -1) {
                auto paragraph_bidi_class = unicode_bidi_P2(first, last);
                paragraph_embedding_level = unicode_bidi_P3(paragraph_bidi_class);
            }

            unicode_bidi_X1(first, last, 0);
            last = unicode_bidi_X9(first, last);
            unicode_bidi_X10(first, last, 0);
            ttlet[lowest_odd, highest] = unicode_bidi_L1(first, last, 0);

            {
                auto it = first;
                for (auto embedding_level: test.levels) {
                    ASSERT_EQ(it->embedding_level, embedding_level);
                    ++it;
                }
            }

            unicode_bidi_L2(first, last, lowest_odd, highest);

            {
                auto it = first;
                for (auto index : test.reorder) {
                    ASSERT_EQ(it->index, index);
                }
            }
        }
    }
}
