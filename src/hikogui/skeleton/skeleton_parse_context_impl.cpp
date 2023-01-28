// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "skeleton_parse_context.hpp"
#include "skeleton_node.hpp"
#include "skeleton_top_node.hpp"
#include "skeleton_do_node.hpp"
#include "skeleton_string_node.hpp"
#include "skeleton.hpp"

namespace hi::inline v1 {

[[nodiscard]] bool skeleton_parse_context::append(std::unique_ptr<skeleton_node> x) noexcept
{
    return statement_stack.back()->append(std::move(x));
}

skeleton_parse_context::skeleton_parse_context(std::filesystem::path const &path, const_iterator first, const_iterator last) :
    location(path), index(first), last(last)
{
    push<skeleton_top_node>(location);
}

std::unique_ptr<formula_node> skeleton_parse_context::parse_expression(std::string_view end_text)
{
    hilet formula_last = find_end_of_formula(index, last, end_text);

    auto context = formula_parse_context(index, formula_last);

    std::unique_ptr<formula_node> expression;

    try {
        expression = parse_formula(context);

    } catch (std::exception const &e) {
        throw parse_error(std::format("{}: Could not parse expression.\n{}", location, e.what()));
    }

    (*this) += std::distance(index, formula_last);
    return expression;
}

std::unique_ptr<formula_node> skeleton_parse_context::parse_expression_and_advance_over(std::string_view end_text)
{
    auto expression = parse_expression(end_text);

    if (!starts_with_and_advance_over(end_text)) {
        throw parse_error(std::format("{}: Could not find '{}' after expression", location, end_text));
    }

    return expression;
}

[[nodiscard]] bool skeleton_parse_context::pop() noexcept
{
    if (statement_stack.size() > 0) {
        auto tmp = std::move(statement_stack.back());
        statement_stack.pop_back();
        return statement_stack.back()->append(std::move(tmp));
    } else {
        return false;
    }
}

[[nodiscard]] bool skeleton_parse_context::top_statement_is_do() const noexcept
{
    if (statement_stack.size() < 1) {
        return false;
    }

    auto const * const ptr = dynamic_cast<skeleton_do_node const *>(statement_stack.back().get());
    return ptr != nullptr;
}

void skeleton_parse_context::start_of_text_segment(int back_track) noexcept
{
    text_segment_start = index - back_track;
}

void skeleton_parse_context::end_of_text_segment()
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

[[nodiscard]] bool skeleton_parse_context::found_elif(parse_location _location, std::unique_ptr<formula_node> expression) noexcept
{
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_elif(std::move(_location), std::move(expression));
    } else {
        return false;
    }
}

[[nodiscard]] bool skeleton_parse_context::found_else(parse_location _location) noexcept
{
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_else(std::move(_location));
    } else {
        return false;
    }
}

[[nodiscard]] bool
skeleton_parse_context::found_while(parse_location _location, std::unique_ptr<formula_node> expression) noexcept
{
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_while(std::move(_location), std::move(expression));
    } else {
        return false;
    }
}

void skeleton_parse_context::include(parse_location _location, formula_node &expression)
{
    auto tmp_post_process_context = formula_post_process_context();
    expression.post_process(tmp_post_process_context);

    auto evaluation_context = formula_evaluation_context();
    hilet argument = expression.evaluate(evaluation_context);

    auto new_skeleton_path = std::filesystem::current_path();
    if (_location.has_file()) {
        // Include relative to the file that is currently parsed.
        new_skeleton_path = _location.file();
        new_skeleton_path.remove_filename();
    }
    new_skeleton_path /= static_cast<std::string>(argument);

    if (ssize(statement_stack) > 0) {
        if (!statement_stack.back()->append(parse_skeleton(new_skeleton_path))) {
            throw parse_error(std::format("{}: Unexpected #include statement.", location));
        }
    } else {
        throw parse_error(std::format("{}: Unexpected #include statement, missing top-level", location));
    }
}

} // namespace hi::inline v1
