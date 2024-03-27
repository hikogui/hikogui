// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_bidi.hpp"
#include "../file/file.hpp"
#include "../path/path.hpp"
#include "../utility/utility.hpp"
#include "../algorithm/algorithm.hpp"
#include <hikotest/hikotest.hpp>
#include <iostream>
#include <string>
#include <string_view>
#include <span>
#include <format>
#include <ranges>

TEST_SUITE(unicode_bidi) {

struct unicode_bidi_test {
    std::vector<int> levels;
    std::vector<int> reorder;
    int line_nr;

    std::vector<hi::unicode_bidi_class> input;
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

    [[nodiscard]] std::vector<hi::unicode_bidi_class> get_paragraph_directions() const noexcept
    {
        auto r = std::vector<hi::unicode_bidi_class>{};

        if (test_for_LTR) {
            r.push_back(hi::unicode_bidi_class::L);
        }
        if (test_for_RTL) {
            r.push_back(hi::unicode_bidi_class::R);
        }
        if (test_for_auto) {
            r.push_back(hi::unicode_bidi_class::ON);
        }

        return r;
    }
};

[[nodiscard]] static std::vector<int> parse_bidi_test_levels(std::string_view line) noexcept
{
    auto r = std::vector<int>{};
    for (auto const value : hi::split(hi::strip(line))) {
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
    for (auto const value : hi::split(hi::strip(line))) {
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

    auto line_s = hi::split(line, ';');

    for (auto bidi_class_str : hi::split(hi::strip(line_s[0]))) {
        r.input.push_back(hi::unicode_bidi_class_from_string(bidi_class_str));
    }

    auto bitset = hi::from_string<int>(hi::strip(line_s[1]), 16);
    r.test_for_auto = (bitset & 1) != 0;
    r.test_for_LTR = (bitset & 2) != 0;
    r.test_for_RTL = (bitset & 4) != 0;

    return r;
}

hi::generator<unicode_bidi_test> parse_bidi_test(int test_line_nr = -1)
{
    auto const view = hi::file_view(hi::library_test_data_dir() / "BidiTest.txt");
    auto const test_data = as_string_view(view);

    auto levels = std::vector<int>{};
    auto reorder = std::vector<int>{};

    int line_nr = 1;
    for (auto const line_view : std::views::split(test_data, std::string_view{"\n"})) {
        auto const line = hi::strip(std::string_view{line_view.begin(), line_view.end()});
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

TEST_CASE(bidi_test)
{
    for (auto test : parse_bidi_test()) {
        for (auto paragraph_direction : test.get_paragraph_directions()) {
            auto test_parameters = hi::unicode_bidi_context{};
            test_parameters.enable_mirrored_brackets = false;
            test_parameters.enable_line_separator = false;
            // clang-format off
            test_parameters.direction_mode =
                paragraph_direction == hi::unicode_bidi_class::L ? hi::unicode_bidi_context::mode_type::LTR :
                paragraph_direction == hi::unicode_bidi_class::R ? hi::unicode_bidi_context::mode_type::RTL :
                hi::unicode_bidi_context::mode_type::auto_LTR;
            // clang-format on

            auto input = test.get_input();
            auto first = begin(input);
            auto last = end(input);

            auto const[new_last, paragraph_directions] = unicode_bidi_P1(first, last, test_parameters);
            last = new_last;

            // We are using the index from the iterator to find embedded levels
            // in input-order. We ignore all elements that where removed by X9.
            for (auto it = first; it != last; ++it) {
                auto const expected_embedding_level = test.levels[it->index];

                REQUIRE((expected_embedding_level == -1 or expected_embedding_level == it->embedding_level));
            }

            REQUIRE(std::distance(first, last) == std::ssize(test.reorder));

            auto index = 0;
            for (auto it = first; it != last; ++it, ++index) {
                auto const expected_input_index = test.reorder[index];

                REQUIRE((expected_input_index == -1 or expected_input_index == it->index));
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
    hi::unicode_bidi_class paragraph_direction;
    hi::unicode_bidi_class resolved_paragraph_direction;
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
        for (auto const c : characters) {
            r.emplace_back(c, index++);
        }

        return r;
    }
};

[[nodiscard]] static unicode_bidi_character_test parse_bidi_character_test_line(std::string_view line, int line_nr)
{
    auto const split_line = hi::split(line, ';');
    auto const hex_characters = hi::split(split_line[0]);
    auto const paragraph_direction = hi::from_string<int>(split_line[1]);
    auto const resolved_paragraph_direction = hi::from_string<int>(split_line[2]);
    auto const int_resolved_levels = hi::split(split_line[3]);
    auto const int_resolved_order = hi::split(split_line[4]);

    auto r = unicode_bidi_character_test{};
    r.line_nr = line_nr;
    std::transform(begin(hex_characters), end(hex_characters), std::back_inserter(r.characters), [](auto const &x) {
        return hi::char_cast<char32_t>(hi::from_string<uint32_t>(x, 16));
    });

    r.paragraph_direction = paragraph_direction == 0 ? hi::unicode_bidi_class::L :
        paragraph_direction == 1                     ? hi::unicode_bidi_class::R :
                                                       hi::unicode_bidi_class::ON;

    r.resolved_paragraph_direction = resolved_paragraph_direction == 0 ? hi::unicode_bidi_class::L :
        resolved_paragraph_direction == 1                              ? hi::unicode_bidi_class::R :
                                                                         hi::unicode_bidi_class::ON;

    std::transform(begin(int_resolved_levels), end(int_resolved_levels), std::back_inserter(r.resolved_levels), [](auto const &x) {
        if (x == "x") {
            return -1;
        } else {
            return hi::from_string<int>(x);
        }
    });

    std::transform(begin(int_resolved_order), end(int_resolved_order), std::back_inserter(r.resolved_order), [](auto const &x) {
        return hi::from_string<int>(x);
    });

    return r;
}

hi::generator<unicode_bidi_character_test> parse_bidi_character_test(int test_line_nr = -1)
{
    auto const view = hi::file_view(hi::library_test_data_dir() / "BidiCharacterTest.txt");
    auto const test_data = as_string_view(view);

    int line_nr = 1;
    for (auto const line_view : std::views::split(test_data, std::string_view{"\n"})) {
        auto const line = hi::strip(std::string_view{line_view.begin(), line_view.end()});
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

TEST_CASE(bidi_character_test)
{
    for (auto test : parse_bidi_character_test()) {
        auto test_parameters = hi::unicode_bidi_context{};
        test_parameters.enable_mirrored_brackets = true;
        test_parameters.enable_line_separator = true;
        // clang-format off
        test_parameters.direction_mode =
            test.paragraph_direction == hi::unicode_bidi_class::L ? hi::unicode_bidi_context::mode_type::LTR :
            test.paragraph_direction == hi::unicode_bidi_class::R ? hi::unicode_bidi_context::mode_type::RTL :
            hi::unicode_bidi_context::mode_type::auto_LTR;
        // clang-format on

        auto input = test.get_input();
        auto first = begin(input);
        auto last = end(input);

        auto const[new_last, paragraph_directions] = hi::unicode_bidi(
            first,
            last,
            [](auto const &x) {
                return x.code_point;
            },
            [](auto &x, auto const &code_point) {
                x.code_point = code_point;
            },
            [](auto &x, auto bidi_class) {},
            test_parameters);

        last = new_last;
        // We are using the index from the iterator to find embedded levels
        // in input-order. We ignore all elements that where removed by X9.
        // for (auto it = first; it != last; ++it) {
        //    auto const expected_embedding_level = test.levels[it->index];
        //
        //    ASSERT_TRUE(expected_embedding_level == -1 || expected_embedding_level == it->embedding_level);
        //}

        REQUIRE(std::distance(first, last) == std::ssize(test.resolved_order));

        auto index = 0;
        for (auto it = first; it != last; ++it, ++index) {
            auto const expected_input_index = test.resolved_order[index];

            REQUIRE((expected_input_index == -1 or expected_input_index == it->index));
        }

#ifndef NDEBUG
        if (test.line_nr > 10'000) {
            break;
        }
#endif
    }
}

};
