// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "stencil_parse_context.hpp"
#include "stencil_node.hpp"
#include "stencil_top_node.hpp"
#include "stencil_do_node.hpp"
#include "stencil_string_node.hpp"
#include "stencil.hpp"

namespace tt {

[[nodiscard]] bool stencil_parse_context::append(std::unique_ptr<stencil_node> x) noexcept
{
    return statement_stack.back()->append(std::move(x));
}

stencil_parse_context::stencil_parse_context(URL const &url, const_iterator first, const_iterator last) :
    location(url), index(first), last(last)
{
    push<stencil_top_node>(location);
}

std::unique_ptr<formula_node> stencil_parse_context::parse_expression(std::string_view end_text) {
    ttlet formula_last = find_end_of_formula(index, last, end_text);

    auto context = formula_parse_context(index, formula_last);

    std::unique_ptr<formula_node> expression;

    try {
        expression = parse_formula(context);
    } catch (error &e) {
        e.merge_location(location);
        throw;
    }

    (*this) += std::distance(index, formula_last);
    return expression;
}

std::unique_ptr<formula_node> stencil_parse_context::parse_expression_and_advance_over(std::string_view end_text) {
    auto expression = parse_expression(end_text);

    if (!starts_with_and_advance_over(end_text)) {
        TTAURI_THROW(parse_error("Could not find '{}' after expression", end_text).set_location(location));
    }

    return expression;
}

[[nodiscard]] bool stencil_parse_context::pop() noexcept {
    if (statement_stack.size() > 0) {
        auto tmp = std::move(statement_stack.back());
        statement_stack.pop_back();
        return statement_stack.back()->append(std::move(tmp));
    } else {
        return false;
    }
}

[[nodiscard]] bool stencil_parse_context::top_statement_is_do() const noexcept
{
    if (statement_stack.size() < 1) {
        return false;
    }

    auto ptr = dynamic_cast<stencil_do_node*>(statement_stack.back().get());
    return ptr != nullptr;
}

void stencil_parse_context::start_of_text_segment(int back_track) noexcept
{
    text_segment_start = index - back_track;
}

void stencil_parse_context::end_of_text_segment()
{
    if (text_segment_start) {
        if (index > *text_segment_start) {
            if (!append<stencil_string_node>(location, std::string(*text_segment_start, index))) {
                TTAURI_THROW(parse_error("Unexpected text segment.").set_location(location));
            }
        }

        text_segment_start = {};
    }
}

[[nodiscard]] bool stencil_parse_context::found_elif(parse_location _location, std::unique_ptr<formula_node> expression) noexcept {
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_elif(std::move(_location), std::move(expression));
    } else {
        return false;
    }
}

[[nodiscard]] bool stencil_parse_context::found_else(parse_location _location) noexcept {
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_else(std::move(_location));
    } else {
        return false;
    }
}

[[nodiscard]] bool stencil_parse_context::found_while(parse_location _location, std::unique_ptr<formula_node> expression) noexcept {
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_while(std::move(_location), std::move(expression));
    } else {
        return false;
    }
}

void stencil_parse_context::include(parse_location _location, std::unique_ptr<formula_node> expression)
{
    auto tmp_post_process_context = formula_post_process_context();
    expression->post_process(tmp_post_process_context);

    auto evaluation_context = formula_evaluation_context();
    ttlet argument = expression->evaluate(evaluation_context);

    ttlet current_stencil_directory = _location.has_file() ?
        _location.file().urlByRemovingFilename() :
        URL::urlFromCurrentWorkingDirectory();

    ttlet new_stencil_path = current_stencil_directory.urlByAppendingPath(static_cast<std::string>(argument));

    if (std::ssize(statement_stack) > 0) {
        if (!statement_stack.back()->append(parse_stencil(new_stencil_path))) {
            TTAURI_THROW(parse_error("Unexpected #include statement").set_location(_location));
        }
    } else {
        TTAURI_THROW(parse_error("Unexpected #include statement, missing top-level").set_location(_location));
    }
}

}

