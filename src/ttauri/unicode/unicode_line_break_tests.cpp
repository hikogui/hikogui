// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/text/unicode_line_break.hpp"
#include "ttauri/text/unicode_description.hpp"
#include "ttauri/file_view.hpp"
#include "ttauri/strings.hpp"
#include "ttauri/coroutine.hpp"
#include "ttauri/ranges.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <span>
#include <format>

namespace {

struct test_type {
    std::u32string code_points;
    std::vector<bool> break_opportunities;
    std::string comment;
    int line_nr;

    [[nodiscard]] bool check() const noexcept
    {
        auto result = tt::detail::unicode_LB(code_points.begin(), code_points.end(), [](ttlet code_point) {
            return tt::unicode_description_find(code_point);
        });

        if (result.size() != break_opportunities.size() - 1) {
            std::cout << "Incorrect result size: " << comment << std::endl;
            return false;
        }

        for (size_t i = 0; i != result.size(); ++i) {
            if (result[i] == tt::unicode_line_break_opportunity::unassigned) {
                std::cout << "unassigned: " << comment << std::endl;
                return false;
            }

            if (break_opportunities[i + 1]) {
                if (result[i] == tt::unicode_line_break_opportunity::no_break) {
                    std::cout << "expected mandatory_break or break_allowed: " << comment << std::endl;
                    return false;
                }

            } else if (result[i] != tt::unicode_line_break_opportunity::no_break) {
                std::cout << "expected no_break: " << comment << std::endl;
                return false;
            }
        }

        return true;
    }
};

static std::optional<test_type> parse_test_line(std::string_view line, int line_nr)
{
    auto r = test_type{};

    ttlet split_line = tt::split(line, "\t#");
    if (split_line.size() < 2) {
        return {};
    }
    r.comment = std::format("{}: {}", line_nr, split_line[1]);
    r.line_nr = line_nr;

    ttlet columns = tt::split(split_line[0]);
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

    return {std::move(r)};
}

static tt::generator<test_type> parse_tests()
{
    ttlet view = tt::file_view(tt::URL("file:LineBreakTest.txt"));
    ttlet test_data = view.string_view();

    int line_nr = 1;
    for (ttlet line : tt::views::split(test_data, "\n")) {
        if (ttlet optional_test = parse_test_line(line, line_nr)) {
            co_yield *optional_test;
        }
        line_nr++;
    }
}

} // namespace

TEST(unicode_line_break, line_break)
{
    for (ttlet &test : parse_tests()) {
        ASSERT_TRUE(test.check());
    }
}
