// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "skeleton_top_node.hpp"
#include "skeleton_string_node.hpp"
#include "skeleton_do_node.hpp"
#include "../algorithm/algorithm.hpp"
#include "../formula/formula.hpp"
#include "../macros.hpp"
#include <memory>
#include <string_view>
#include <optional>
#include <filesystem>

hi_export_module(hikogui.skeleton : parse_context);

hi_export namespace hi::inline v1 {

[[nodiscard]] std::unique_ptr<skeleton_node> parse_skeleton(std::filesystem::path path);

struct skeleton_parse_context {
    using statement_stack_type = std::vector<std::unique_ptr<skeleton_node>>;
    using const_iterator = typename std::string_view::const_iterator;

    statement_stack_type statement_stack;

    parse_location location;
    const_iterator index;
    const_iterator last;

    std::optional<const_iterator> text_segment_start;

    /** Post process context is used to record functions that are defined
     * in the template being parsed.
     */
    formula_post_process_context post_process_context;

    skeleton_parse_context() = delete;
    skeleton_parse_context(skeleton_parse_context const& other) = delete;
    skeleton_parse_context& operator=(skeleton_parse_context const& other) = delete;
    skeleton_parse_context(skeleton_parse_context&& other) = delete;
    skeleton_parse_context& operator=(skeleton_parse_context&& other) = delete;
    ~skeleton_parse_context() = default;

    hi_inline skeleton_parse_context(std::filesystem::path const& path, const_iterator first, const_iterator last) :
        location(path.string()), index(first), last(last)
    {
        push<skeleton_top_node>(location);
    }

    [[nodiscard]] constexpr char const& operator*() const noexcept
    {
        return *index;
    }

    [[nodiscard]] constexpr bool atEOF() const noexcept
    {
        return index == last;
    }

    constexpr skeleton_parse_context& operator++() noexcept
    {
        hi_assert(not atEOF());
        location += *index;
        ++index;
        return *this;
    }

    constexpr skeleton_parse_context& operator+=(ssize_t x) noexcept
    {
        for (ssize_t i = 0; i != x; ++i) {
            ++(*this);
        }
        return *this;
    }

    constexpr bool starts_with(std::string_view text) const noexcept
    {
        return std::string_view{index, last}.starts_with(text);
    }

    constexpr bool starts_with_and_advance_over(std::string_view text) noexcept
    {
        if (starts_with(text)) {
            *this += ssize(text);
            return true;
        } else {
            return false;
        }
    }

    constexpr bool advance_to(std::string_view text) noexcept
    {
        while (!atEOF()) {
            if (starts_with(text)) {
                return true;
            }
            ++(*this);
        }
        return false;
    }

    constexpr bool advance_over(std::string_view text) noexcept
    {
        if (advance_to(text)) {
            *this += ssize(text);
            return true;
        } else {
            return false;
        }
    }

    constexpr std::unique_ptr<formula_node> parse_expression(std::string_view end_text)
    {
        hilet formula_last = find_end_of_formula(index, last, end_text);

        std::unique_ptr<formula_node> expression;

        try {
            expression = parse_formula_without_post_processing(std::string_view{index, formula_last});

        } catch (std::exception const& e) {
            throw parse_error(std::format("{}: Could not parse expression.\n{}", location, e.what()));
        }

        (*this) += std::distance(index, formula_last);
        return expression;
    }

    constexpr std::unique_ptr<formula_node> parse_expression_and_advance_over(std::string_view end_text)
    {
        auto expression = parse_expression(end_text);

        if (!starts_with_and_advance_over(end_text)) {
            throw parse_error(std::format("{}: Could not find '{}' after expression", location, end_text));
        }

        return expression;
    }

    template<typename T, typename... Args>
    constexpr void push(Args&&...args)
    {
        statement_stack.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    [[nodiscard]] constexpr bool append(std::unique_ptr<skeleton_node> x) noexcept
    {
        return statement_stack.back()->append(std::move(x));
    }

    template<typename T, typename... Args>
    [[nodiscard]] constexpr bool append(Args&&...args) noexcept
    {
        if (statement_stack.size() > 0) {
            return append(std::make_unique<T>(std::forward<Args>(args)...));
        } else {
            return false;
        }
    }

    /** Handle \#end statement.
     * This will pop the current statement of the stack and append it
     * to the statement that is now at the top of the stack.
     */
    [[nodiscard]] constexpr bool pop() noexcept
    {
        if (statement_stack.size() > 0) {
            auto tmp = std::move(statement_stack.back());
            statement_stack.pop_back();
            return statement_stack.back()->append(std::move(tmp));
        } else {
            return false;
        }
    }

    constexpr void start_of_text_segment(int back_track = 0) noexcept
    {
        text_segment_start = index - back_track;
    }

    constexpr void end_of_text_segment()
    {
        if (text_segment_start) {
            if (index > *text_segment_start) {
                if (!append<skeleton_string_node>(location, std::string(*text_segment_start, index))) {
                    throw parse_error(std::format("{}: Unexpected text segment.", location));
                }
            }

            text_segment_start = {};
        }
    }

    [[nodiscard]] constexpr bool top_statement_is_do() const noexcept
    {
        if (statement_stack.size() < 1) {
            return false;
        }

        auto const *const ptr = dynamic_cast<skeleton_do_node const *>(statement_stack.back().get());
        return ptr != nullptr;
    }

    [[nodiscard]] constexpr bool found_elif(parse_location statement_location, std::unique_ptr<formula_node> expression) noexcept
    {
        if (statement_stack.size() > 0) {
            return statement_stack.back()->found_elif(std::move(statement_location), std::move(expression));
        } else {
            return false;
        }
    }

    [[nodiscard]] constexpr bool found_else(parse_location statement_location) noexcept
    {
        if (statement_stack.size() > 0) {
            return statement_stack.back()->found_else(std::move(statement_location));
        } else {
            return false;
        }
    }

    [[nodiscard]] constexpr bool found_while(parse_location statement_location, std::unique_ptr<formula_node> expression) noexcept
    {
        if (statement_stack.size() > 0) {
            return statement_stack.back()->found_while(std::move(statement_location), std::move(expression));
        } else {
            return false;
        }
    }

    hi_inline void include(parse_location statement_location, formula_node& expression)
    {
        auto tmp_post_process_context = formula_post_process_context();
        expression.post_process(tmp_post_process_context);

        auto evaluation_context = formula_evaluation_context();
        hilet argument = expression.evaluate(evaluation_context);

        auto new_skeleton_path = std::filesystem::current_path();
        if (statement_location.has_file()) {
            // Include relative to the file that is currently parsed.
            new_skeleton_path = statement_location.file();
            new_skeleton_path.remove_filename();
        }
        new_skeleton_path /= static_cast<std::string>(argument);

        if (ssize(statement_stack) > 0) {
            if (!statement_stack.back()->append(parse_skeleton(new_skeleton_path))) {
                throw parse_error(std::format("{}: Unexpected #include statement.", statement_location));
            }
        } else {
            throw parse_error(std::format("{}: Unexpected #include statement, missing top-level", statement_location));
        }
    }
};

} // namespace hi::inline v1
