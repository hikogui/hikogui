// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "veer_parse_context.hpp"
#include "veer_node.hpp"
#include "veer_top_node.hpp"
#include "veer_do_node.hpp"
#include "veer_string_node.hpp"
#include "veer.hpp"

namespace tt {

[[nodiscard]] bool veer_parse_context::append(std::unique_ptr<veer_node> x) noexcept
{
    return statement_stack.back()->append(std::move(x));
}

veer_parse_context::veer_parse_context(URL const &url, const_iterator first, const_iterator last) :
    location(url), index(first), last(last)
{
    push<veer_top_node>(location);
}

std::unique_ptr<expression_node> veer_parse_context::parse_expression(std::string_view end_text) {
    ttlet expression_last = find_end_of_expression(index, last, end_text);

    auto context = expression_parse_context(index, expression_last);

    std::unique_ptr<expression_node> expression;

    try {
        expression = ::tt::parse_expression(context);
    } catch (error &e) {
        e.merge_location(location);
        throw;
    }

    (*this) += std::distance(index, expression_last);
    return expression;
}

std::unique_ptr<expression_node> veer_parse_context::parse_expression_and_advance_over(std::string_view end_text) {
    auto expression = parse_expression(end_text);

    if (!starts_with_and_advance_over(end_text)) {
        TTAURI_THROW(parse_error("Could not find '{}' after expression", end_text).set_location(location));
    }

    return expression;
}

[[nodiscard]] bool veer_parse_context::pop() noexcept {
    if (statement_stack.size() > 0) {
        auto tmp = std::move(statement_stack.back());
        statement_stack.pop_back();
        return statement_stack.back()->append(std::move(tmp));
    } else {
        return false;
    }
}

[[nodiscard]] bool veer_parse_context::top_statement_is_do() const noexcept
{
    if (statement_stack.size() < 1) {
        return false;
    }

    auto ptr = dynamic_cast<veer_do_node*>(statement_stack.back().get());
    return ptr != nullptr;
}

void veer_parse_context::start_of_text_segment(int back_track) noexcept
{
    text_segment_start = index - back_track;
}

void veer_parse_context::end_of_text_segment()
{
    if (text_segment_start) {
        if (index > *text_segment_start) {
            if (!append<veer_string_node>(location, std::string(*text_segment_start, index))) {
                TTAURI_THROW(parse_error("Unexpected text segment.").set_location(location));
            }
        }

        text_segment_start = {};
    }
}

[[nodiscard]] bool veer_parse_context::found_elif(parse_location _location, std::unique_ptr<expression_node> expression) noexcept {
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_elif(std::move(_location), std::move(expression));
    } else {
        return false;
    }
}

[[nodiscard]] bool veer_parse_context::found_else(parse_location _location) noexcept {
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_else(std::move(_location));
    } else {
        return false;
    }
}

[[nodiscard]] bool veer_parse_context::found_while(parse_location _location, std::unique_ptr<expression_node> expression) noexcept {
    if (statement_stack.size() > 0) {
        return statement_stack.back()->found_while(std::move(_location), std::move(expression));
    } else {
        return false;
    }
}

void veer_parse_context::include(parse_location _location, std::unique_ptr<expression_node> expression)
{
    auto tmp_post_process_context = expression_post_process_context();
    expression->post_process(tmp_post_process_context);

    auto evaluation_context = expression_evaluation_context();
    ttlet argument = expression->evaluate(evaluation_context);

    ttlet current_veer_directory = _location.has_file() ?
        _location.file().urlByRemovingFilename() :
        URL::urlFromCurrentWorkingDirectory();

    ttlet new_veer_path = current_veer_directory.urlByAppendingPath(static_cast<std::string>(argument));

    if (std::ssize(statement_stack) > 0) {
        if (!statement_stack.back()->append(parse_veer(new_veer_path))) {
            TTAURI_THROW(parse_error("Unexpected #include statement").set_location(_location));
        }
    } else {
        TTAURI_THROW(parse_error("Unexpected #include statement, missing top-level").set_location(_location));
    }
}

}

