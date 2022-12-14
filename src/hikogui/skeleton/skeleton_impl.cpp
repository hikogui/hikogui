// Copyright Take Vos 2020, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "skeleton.hpp"
#include "skeleton_node.hpp"
#include "skeleton_block_node.hpp"
#include "skeleton_break_node.hpp"
#include "skeleton_continue_node.hpp"
#include "skeleton_do_node.hpp"
#include "skeleton_expression_node.hpp"
#include "skeleton_for_node.hpp"
#include "skeleton_function_node.hpp"
#include "skeleton_if_node.hpp"
#include "skeleton_placeholder_node.hpp"
#include "skeleton_return_node.hpp"
#include "skeleton_string_node.hpp"
#include "skeleton_top_node.hpp"
#include "skeleton_while_node.hpp"
#include "../formula/formula.hpp"
#include "../strings.hpp"
#include "../algorithm.hpp"

namespace hi::inline v1 {

static void parse_skeleton_hash(skeleton_parse_context &context)
{
    hilet &location = context.location;

    if (context.starts_with("end")) {
        context.advance_over("\n");

        if (!context.pop()) {
            throw parse_error(std::format("{}: Unexpected #end statement.", location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("if ")) {
        context.push<skeleton_if_node>(location, context.parse_expression_and_advance_over("\n"));

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("elif ")) {
        if (!context.found_elif(location, context.parse_expression_and_advance_over("\n"))) {
            throw parse_error(std::format("{}: Unexpected #elif statement.", location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("else")) {
        context.advance_over("\n");

        if (!context.found_else(location)) {
            throw parse_error(std::format("{}: Unexpected #else statement.", location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("for ")) {
        auto name_expression = context.parse_expression_and_advance_over(":");
        auto list_expression = context.parse_expression_and_advance_over("\n");

        context.push<skeleton_for_node>(location, std::move(name_expression), std::move(list_expression));

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("while ")) {
        auto expression = context.parse_expression_and_advance_over("\n");

        if (context.top_statement_is_do()) {
            if (!context.found_while(location, std::move(expression))) {
                throw parse_error(std::format("{}: Unexpected #while statement; missing #do.", location));
            }

            hi_assert(context.pop());
        } else {
            context.push<skeleton_while_node>(location, std::move(expression));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("do")) {
        context.advance_over("\n");

        context.push<skeleton_do_node>(location);

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("function ")) {
        context.push<skeleton_function_node>(
            location, context.post_process_context, context.parse_expression_and_advance_over("\n"));

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("block ")) {
        context.push<skeleton_block_node>(
            location, context.post_process_context, context.parse_expression_and_advance_over("\n"));

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("break")) {
        context.advance_over("\n");

        if (!context.append<skeleton_break_node>(location)) {
            throw parse_error(std::format("{}: Unexpected #break statement", location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("continue")) {
        context.advance_over("\n");

        if (!context.append<skeleton_continue_node>(location)) {
            throw parse_error(std::format("{}: Unexpected #continue statement", location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("return ")) {
        if (!context.append<skeleton_return_node>(location, context.parse_expression_and_advance_over("\n"))) {
            throw parse_error(std::format("{}: Unexpected #return statement", location));
        }

        context.start_of_text_segment();

    } else if (context.starts_with_and_advance_over("include ")) {
        context.include(location, context.parse_expression_and_advance_over("\n"));
        context.start_of_text_segment();

    } else { // Add '#' and the current character to text.
        if (!context.append<skeleton_expression_node>(location, context.parse_expression_and_advance_over("\n"))) {
            throw parse_error(std::format("{}: Unexpected # (expression) statement.", location));
        }

        context.start_of_text_segment();
    }
}

static void parse_skeleton_dollar(skeleton_parse_context &context)
{
    hilet &location = context.location;

    if (*context == '{') {
        ++context;
        if (!context.append<skeleton_placeholder_node>(location, context.parse_expression_and_advance_over("}"))) {
            throw parse_error("Unexpected placeholder.");
        }

        context.start_of_text_segment();

    } else {
        ++context;
        context.start_of_text_segment(-2);
    }
}

static void parse_skeleton_escape(skeleton_parse_context &context)
{
    for (; !context.atEOF(); ++context) {
        hilet c = *context;

        switch (c) {
        case '\n': // Skip over line-feed
            ++context;
            context.start_of_text_segment();
            return;

        case '\r': // Skip over carriage return and potential line-feed.
            ++context;
            break;

        default: // Add character to text.
            ++context;
            context.start_of_text_segment(-2);
            return;
        }
    }
    throw parse_error(std::format("{}: Unexpected end-of-file after escape '\' character.", context.location));
}

[[nodiscard]] std::unique_ptr<skeleton_node> parse_skeleton(skeleton_parse_context &context)
{
    context.start_of_text_segment();

    while (!context.atEOF()) {
        switch (*context) {
        case '#':
            context.end_of_text_segment();
            ++context;
            parse_skeleton_hash(context);
            break;

        case '$':
            context.end_of_text_segment();
            ++context;
            parse_skeleton_dollar(context);
            break;

        case '\\': // Skip the backslash.
            context.end_of_text_segment();
            ++context;
            parse_skeleton_escape(context);
            break;

        default: ++context;
        }
    }
    context.end_of_text_segment();

    if (context.statement_stack.size() < 1) {
        throw parse_error(std::format("{}: Found to many #end statements.", context.location));
    } else if (context.statement_stack.size() > 1) {
        throw parse_error(std::format("{}: Missing #end statement.", context.location));
    }

    auto top = std::move(context.statement_stack.back());
    context.statement_stack.pop_back();

    top->post_process(context.post_process_context);
    return top;
}

} // namespace hi::inline v1
