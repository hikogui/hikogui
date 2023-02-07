// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_bidi.hpp"
#include "../file/file_view.hpp"
#include "../utility/module.hpp"
#include "../ranges.hpp"
#include "../strings.hpp"
#include "../generator.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <string_view>
#include <span>
#include <format>
#include <ranges>

using namespace hi;

struct unicode_bidi_test {
    std::vector<int> levels;
    std::vector<int> reorder;
    int line_nr;

    std::vector<unicode_bidi_class> input;
    bool test_for_LTR = false;
    bool test_for_RTL = false;
    bool test_for_auto = false;

    [[nodiscard]] unicode_bidi_test(std::vector<int> const &levels, std::vector<int> const &reorder, int line_nr) noexcept :
        levels(levels), reorder(reorder), line_nr(line_nr)
    {
    }

    [[nodiscard]] std::vector<hi::detail::unicode_bidi_char_info> get_input() const noexcept
    {
        auto r = std::vector<hi::detail::unicode_bidi_char_info>{};
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
    for (hilet value : hi::split(hi::strip(line))) {
        if (value == "x") {
            r.push_back(-1);
        } else {
            r.push_back(hi::from_string<int>(value));
        }
    }
    return r;
}

[[nodiscard]] static std::vector<int> parse_bidi_test_reorder(std::string_view line) noexcept
{
    auto r = std::vector<int>{};
    for (hilet value : split(strip(line))) {
        if (value == "x") {
            r.push_back(-1);
        } else {
            r.push_back(hi::from_string<int>(value));
        }
    }
    return r;
}

[[nodiscard]] static unicode_bidi_test parse_bidi_test_data_line(
    std::string_view line,
    std::vector<int> const &levels,
    std::vector<int> const &reorder,
    int level_nr) noexcept
{
    auto r = unicode_bidi_test{levels, reorder, level_nr};

    auto line_s = split(line, ';');

    for (auto bidi_class_str : split(strip(line_s[0]))) {
        r.input.push_back(unicode_bidi_class_from_string(bidi_class_str));
    }

    auto bitset = hi::from_string<int>(strip(line_s[1]), 16);
    r.test_for_auto = (bitset & 1) != 0;
    r.test_for_LTR = (bitset & 2) != 0;
    r.test_for_RTL = (bitset & 4) != 0;

    return r;
}

generator<unicode_bidi_test> parse_bidi_test(int test_line_nr = -1)
{
    hilet view = file_view("BidiTest.txt");
    hilet test_data = as_string_view(view);

    auto levels = std::vector<int>{};
    auto reorder = std::vector<int>{};

    int line_nr = 1;
    for (hilet line_view : std::views::split(test_data, std::string_view{"\n"})) {
        hilet line = strip(std::string_view{line_view.begin(), line_view.end()});
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

TEST(unicode_bidi, bidi_test)
{
    for (auto test : parse_bidi_test()) {
        for (auto paragraph_direction : test.get_paragraph_directions()) {
            auto test_parameters = hi::unicode_bidi_context{};
            test_parameters.enable_mirrored_brackets = false;
            test_parameters.enable_line_separator = false;
            // clang-format off
            test_parameters.direction_mode =
                paragraph_direction == unicode_bidi_class::L ? hi::unicode_bidi_context::mode_type::LTR :
                paragraph_direction == unicode_bidi_class::R ? hi::unicode_bidi_context::mode_type::RTL :
                hi::unicode_bidi_context::mode_type::auto_LTR;
            // clang-format on

            auto input = test.get_input();
            auto first = begin(input);
            auto last = end(input);

            hilet[new_last, paragraph_directions] = unicode_bidi_P1(first, last, test_parameters);
            last = new_last;

            // We are using the index from the iterator to find embedded levels
            // in input-order. We ignore all elements that where removed by X9.
            for (auto it = first; it != last; ++it) {
                hilet expected_embedding_level = test.levels[it->index];

                ASSERT_TRUE(expected_embedding_level == -1 || expected_embedding_level == it->embedding_level);
            }

            ASSERT_EQ(std::distance(first, last), ssize(test.reorder));

            auto index = 0;
            for (auto it = first; it != last; ++it, ++index) {
                hilet expected_input_index = test.reorder[index];

                ASSERT_TRUE(expected_input_index == -1 || expected_input_index == it->index);
            }
        }

#ifndef NDEBUG
        if (test.line_nr > 10'000) {
            break;
        }
#endif
    }
}

struct unicode_bidi_character_test {
    int line_nr;
    std::vector<char32_t> characters;
    unicode_bidi_class paragraph_direction;
    unicode_bidi_class resolved_paragraph_direction;
    std::vector<int> resolved_levels;
    std::vector<int> resolved_order;

    struct input_character {
        char32_t code_point;
        int index;
    };

    [[nodiscard]] std::vector<input_character> get_input() const noexcept
    {
        auto r = std::vector<input_character>{};

        int index = 0;
        for (hilet c : characters) {
            r.emplace_back(c, index++);
        }

        return r;
    }
};

[[nodiscard]] static unicode_bidi_character_test parse_bidi_character_test_line(std::string_view line, int line_nr)
{
    hilet split_line = split(line, ';');
    hilet hex_characters = split(split_line[0]);
    hilet paragraph_direction = hi::from_string<int>(split_line[1]);
    hilet resolved_paragraph_direction = hi::from_string<int>(split_line[2]);
    hilet int_resolved_levels = split(split_line[3]);
    hilet int_resolved_order = split(split_line[4]);

    auto r = unicode_bidi_character_test{};
    r.line_nr = line_nr;
    std::transform(begin(hex_characters), end(hex_characters), std::back_inserter(r.characters), [](hilet &x) {
        return char_cast<char32_t>(hi::from_string<uint32_t>(x, 16));
    });

    r.paragraph_direction = paragraph_direction == 0 ? unicode_bidi_class::L :
        paragraph_direction == 1                     ? unicode_bidi_class::R :
                                                       unicode_bidi_class::unknown;

    r.resolved_paragraph_direction = resolved_paragraph_direction == 0 ? unicode_bidi_class::L :
        resolved_paragraph_direction == 1                              ? unicode_bidi_class::R :
                                                                         unicode_bidi_class::unknown;

    std::transform(begin(int_resolved_levels), end(int_resolved_levels), std::back_inserter(r.resolved_levels), [](hilet &x) {
        if (x == "x") {
            return -1;
        } else {
            return hi::from_string<int>(x);
        }
    });

    std::transform(begin(int_resolved_order), end(int_resolved_order), std::back_inserter(r.resolved_order), [](hilet &x) {
        return hi::from_string<int>(x);
    });

    return r;
}

generator<unicode_bidi_character_test> parse_bidi_character_test(int test_line_nr = -1)
{
    hilet view = file_view("BidiCharacterTest.txt");
    hilet test_data = as_string_view(view);

    int line_nr = 1;
    for (hilet line_view : std::views::split(test_data, std::string_view{"\n"})) {
        hilet line = strip(std::string_view{line_view.begin(), line_view.end()});
        if (line.empty() || line.starts_with("#")) {
            // Comment and empty lines.
        } else {
            auto data = parse_bidi_character_test_line(line, line_nr);
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

TEST(unicode_bidi, bidi_character_test)
{
    for (auto test : parse_bidi_character_test()) {
        auto test_parameters = hi::unicode_bidi_context{};
        test_parameters.enable_mirrored_brackets = true;
        test_parameters.enable_line_separator = true;
        // clang-format off
        test_parameters.direction_mode =
            test.paragraph_direction == unicode_bidi_class::L ? hi::unicode_bidi_context::mode_type::LTR :
            test.paragraph_direction == unicode_bidi_class::R ? hi::unicode_bidi_context::mode_type::RTL :
            hi::unicode_bidi_context::mode_type::auto_LTR;
        // clang-format on

        auto input = test.get_input();
        auto first = begin(input);
        auto last = end(input);

        hilet[new_last, paragraph_directions] = unicode_bidi(
            first,
            last,
            [](hilet &x) {
                return std::make_pair(x.code_point, &unicode_description::find(x.code_point));
            },
            [](auto &x, hilet &code_point) {
                x.code_point = code_point;
            },
            [](auto &x, auto bidi_class) {},
            test_parameters);

        last = new_last;
        // We are using the index from the iterator to find embedded levels
        // in input-order. We ignore all elements that where removed by X9.
        // for (auto it = first; it != last; ++it) {
        //    hilet expected_embedding_level = test.levels[it->index];
        //
        //    ASSERT_TRUE(expected_embedding_level == -1 || expected_embedding_level == it->embedding_level);
        //}

        ASSERT_EQ(std::distance(first, last), ssize(test.resolved_order));

        auto index = 0;
        for (auto it = first; it != last; ++it, ++index) {
            hilet expected_input_index = test.resolved_order[index];

            ASSERT_TRUE(expected_input_index == -1 || expected_input_index == it->index);
        }

#ifndef NDEBUG
        if (test.line_nr > 10'000) {
            break;
        }
#endif
    }
}
