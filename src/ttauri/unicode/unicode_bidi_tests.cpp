// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/text/unicode_bidi.hpp"
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

using namespace tt;

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

    [[nodiscard]] std::vector<tt::detail::unicode_bidi_char_info> get_input() const noexcept
    {
        auto r = std::vector<tt::detail::unicode_bidi_char_info>{};
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

    auto bitset = tt::from_string<int>(strip(line_s[1]), 16);
    r.test_for_auto = (bitset & 1) != 0;
    r.test_for_LTR = (bitset & 2) != 0;
    r.test_for_RTL = (bitset & 4) != 0;

    return r;
}

generator<unicode_bidi_test> parse_bidi_test(int test_line_nr = -1)
{
    ttlet view = file_view(URL("file:BidiTest.txt"));
    ttlet test_data = view.string_view();

    auto levels = std::vector<int>{};
    auto reorder = std::vector<int>{};

    int line_nr = 1;
    for (ttlet line : tt::views::split(test_data, "\n")) {
        ttlet line_ = strip(line);
        if (line_.empty() || line_.starts_with("#")) {
            // Comment and empty lines.
        } else if (line_.starts_with("@Levels:")) {
            levels = parse_bidi_test_levels(line_.substr(8));
        } else if (line_.starts_with("@Reorder:")) {
            reorder = parse_bidi_test_reorder(line_.substr(9));
        } else {
            auto data = parse_bidi_test_data_line(line_, levels, reorder, line_nr);
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
            auto test_parameters = tt::unicode_bidi_context{};
            test_parameters.enable_mirrored_brackets = false;
            test_parameters.enable_line_separator = false;
            test_parameters.default_paragraph_direction = paragraph_direction;
            test_parameters.move_lf_and_ps_to_end_of_line = false;

            auto input = test.get_input();
            auto first = begin(input);
            auto last = end(input);

            ttlet [new_last, paragraph_directions] = unicode_bidi_P1(first, last, test_parameters);
            last = new_last;

            // We are using the index from the iterator to find embedded levels
            // in input-order. We ignore all elements that where removed by X9.
            for (auto it = first; it != last; ++it) {
                ttlet expected_embedding_level = test.levels[it->index];

                ASSERT_TRUE(expected_embedding_level == -1 || expected_embedding_level == it->embedding_level);
            }

            ASSERT_EQ(std::distance(first, last), ssize(test.reorder));

            auto index = 0;
            for (auto it = first; it != last; ++it, ++index) {
                ttlet expected_input_index = test.reorder[index];

                ASSERT_TRUE(expected_input_index == -1 || expected_input_index == it->index);
            }
        }

        if constexpr (build_type::current == build_type::debug) {
            if (test.line_nr > 10'000) {
                break;
            }
        }
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
        for (ttlet c : characters) {
            r.emplace_back(c, index++);
        }

        return r;
    }
};

[[nodiscard]] static unicode_bidi_character_test parse_bidi_character_test_line(std::string_view line, int line_nr)
{
    ttlet split_line = split(line, ';');
    ttlet hex_characters = split(split_line[0]);
    ttlet paragraph_direction = tt::from_string<int>(split_line[1]);
    ttlet resolved_paragraph_direction = tt::from_string<int>(split_line[2]);
    ttlet int_resolved_levels = split(split_line[3]);
    ttlet int_resolved_order = split(split_line[4]);

    auto r = unicode_bidi_character_test{};
    r.line_nr = line_nr;
    std::transform(begin(hex_characters), end(hex_characters), std::back_inserter(r.characters), [](ttlet &x) {
        return static_cast<char32_t>(tt::from_string<uint32_t>(x, 16));
    });

    r.paragraph_direction = paragraph_direction == 0 ? unicode_bidi_class::L :
        paragraph_direction == 1                     ? unicode_bidi_class::R :
                                                       unicode_bidi_class::unknown;

    r.resolved_paragraph_direction = resolved_paragraph_direction == 0 ? unicode_bidi_class::L :
        resolved_paragraph_direction == 1                              ? unicode_bidi_class::R :
                                                                         unicode_bidi_class::unknown;

    std::transform(begin(int_resolved_levels), end(int_resolved_levels), std::back_inserter(r.resolved_levels), [](ttlet &x) {
        if (x == "x") {
            return -1;
        } else {
            return tt::from_string<int>(x);
        }
    });

    std::transform(begin(int_resolved_order), end(int_resolved_order), std::back_inserter(r.resolved_order), [](ttlet &x) {
        return tt::from_string<int>(x);
    });

    return r;
}

generator<unicode_bidi_character_test> parse_bidi_character_test(int test_line_nr = -1)
{
    ttlet view = file_view(URL("file:BidiCharacterTest.txt"));
    ttlet test_data = view.string_view();

    int line_nr = 1;
    for (ttlet line : tt::views::split(test_data, "\n")) {
        ttlet line_ = strip(line);
        if (line_.empty() || line_.starts_with("#")) {
            // Comment and empty lines.
        } else {
            auto data = parse_bidi_character_test_line(line_, line_nr);
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
        auto test_parameters = tt::unicode_bidi_context{};
        test_parameters.enable_mirrored_brackets = true;
        test_parameters.enable_line_separator = true;
        test_parameters.default_paragraph_direction = test.paragraph_direction;
        test_parameters.move_lf_and_ps_to_end_of_line = false;

        auto input = test.get_input();
        auto first = begin(input);
        auto last = end(input);

        ttlet [new_last, paragraph_directions] = unicode_bidi(
            first,
            last,
            [](ttlet &x) {
                return x.code_point;
            },
            [](auto &x, ttlet &code_point) {
                x.code_point = code_point;
            },
            [](auto &x, auto bidi_class) {},
            test_parameters);
        last = new_last;
        // We are using the index from the iterator to find embedded levels
        // in input-order. We ignore all elements that where removed by X9.
        // for (auto it = first; it != last; ++it) {
        //    ttlet expected_embedding_level = test.levels[it->index];
        //
        //    ASSERT_TRUE(expected_embedding_level == -1 || expected_embedding_level == it->embedding_level);
        //}

        ASSERT_EQ(std::distance(first, last), ssize(test.resolved_order));

        auto index = 0;
        for (auto it = first; it != last; ++it, ++index) {
            ttlet expected_input_index = test.resolved_order[index];

            ASSERT_TRUE(expected_input_index == -1 || expected_input_index == it->index);
        }

        if constexpr (build_type::current == build_type::debug) {
            if (test.line_nr > 10'000) {
                break;
            }
        }
    }
}
