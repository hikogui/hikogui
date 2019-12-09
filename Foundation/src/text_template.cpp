// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/text_template.hpp"

namespace TTauri {

struct template_string_node: template_node {
    std::string text;

    template_string_node(ssize_t offset, std::string text) :
        template_node(offset), text(std::move(text)) {}

    std::string string() const noexcept override {
        return fmt::format("<text {}>", text);
    }
};

struct template_placeholder_node: template_node {
    std::unique_ptr<expression_node> expression;

    template_placeholder_node(ssize_t offset, std::unique_ptr<expression_node> expression) :
        template_node(offset), expression(std::move(expression)) {}

    std::string string() const noexcept override {
        return fmt::format("<placeholder {}>", *expression);
    }
};

struct template_if_node: template_node {
    std::vector<statement_vector> children_groups;
    std::vector<std::unique_ptr<expression_node>> expressions;
    std::vector<ssize_t> expression_offsets;

    template_if_node(ssize_t offset, std::unique_ptr<expression_node> expression) noexcept :
        template_node(offset)
    {
        expressions.push_back(std::move(expression));
        expression_offsets.push_back(offset);
        children_groups.emplace_back();
    }

    bool found_elif(ssize_t offset, std::unique_ptr<expression_node> expression) noexcept override {
        if (children_groups.size() != expressions.size()) {
            return false;
        }

        expressions.push_back(std::move(expression));
        expression_offsets.push_back(offset);
        children_groups.emplace_back();
        return true;
    }

    bool found_else(ssize_t offset) noexcept override {
        if (children_groups.size() != expressions.size()) {
            return false;
        }

        expression_offsets.push_back(offset);
        children_groups.emplace_back();
        return true;
    }

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        children_groups.back().push_back(std::move(x));
        return true;
    }

    std::string string() const noexcept override {
        required_assert(expressions.size() > 0);
        std::string s = "<if ";
        s += to_string(*expressions[0]);
        s += ": ";
        s += join(transform<std::vector<std::string>>(children_groups[0], [](auto &x) { return to_string(*x); }));

        for (size_t i = 1; i != expressions.size(); ++i) {
            s += "elif ";
            s += to_string(*expressions[i]);
            s += ": ";
            s += join(transform<std::vector<std::string>>(children_groups[i], [](auto &x) { return to_string(*x); }));
        }

        if (children_groups.size() != expressions.size()) {
            s += "else: ";
            s += join(transform<std::vector<std::string>>(children_groups.back(), [](auto &x) { return to_string(*x); }));
        }

        s += ">";
        return s;
    }
};

struct template_while_node: template_node {
    statement_vector children;
    std::unique_ptr<expression_node> expression;

    template_while_node(ssize_t offset, std::unique_ptr<expression_node> expression) noexcept :
        template_node(offset), expression(std::move(expression)) {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        children.push_back(std::move(x));
        return true;
    }

    std::string string() const noexcept override {
        std::string s = "<while ";
        s += to_string(*expression);
        s += ": ";
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += ">";
        return s;
    }
};

struct template_do_node: template_node {
    statement_vector children;
    std::unique_ptr<expression_node> expression;
    ssize_t expression_offset;

    template_do_node(ssize_t offset) noexcept :
        template_node(offset) {}


    bool found_while(ssize_t offset, std::unique_ptr<expression_node> x) noexcept override {
        if (expression) {
            return false;
        } else {
            expression = std::move(x);
            expression_offset = offset;
            return true;
        }
    }

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        if (expression) {
            return false;
        } else {
            children.push_back(std::move(x));
            return true;
        }
    }

    std::string string() const noexcept override {
        required_assert(expression);
        std::string s = "<do ";
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += ": ";
        s += to_string(*expression);
        s += ">";
        return s;
    }
};

struct template_for_node: template_node {
    std::unique_ptr<expression_node> name_expression;
    std::unique_ptr<expression_node> list_expression;
    statement_vector children;

    template_for_node(ssize_t offset, std::unique_ptr<expression_node> name_expression, std::unique_ptr<expression_node> list_expression) noexcept :
        template_node(offset), name_expression(std::move(name_expression)), list_expression(std::move(list_expression)) {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        children.push_back(std::move(x));
        return true;
    }

    std::string string() const noexcept override {
        std::string s = "<for ";
        s += to_string(*name_expression);
        s += " in ";
        s += to_string(*list_expression);
        s += ": ";
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += ">";
        return s;
    }
};

struct template_function_node: template_node {
    std::unique_ptr<expression_node> name_expression;
    statement_vector children;

    template_function_node(ssize_t offset, std::unique_ptr<expression_node> name_expression) noexcept :
        template_node(offset), name_expression(std::move(name_expression)) {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        children.push_back(std::move(x));
        return true;
    }

    std::string string() const noexcept override {
        std::string s = "<function ";
        s += to_string(*name_expression);
        s += ": ";
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += ">";
        return s;
    }

};

struct template_block_node: template_node {
    std::unique_ptr<expression_node> name_expression;
    statement_vector children;

    template_block_node(ssize_t offset, std::unique_ptr<expression_node> name_expression) noexcept :
        template_node(offset), name_expression(std::move(name_expression)) {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<template_node> x) noexcept override {
        children.push_back(std::move(x));
        return true;
    }

    std::string string() const noexcept override {
        std::string s = "<block ";
        s += to_string(*name_expression);
        s += ": ";
        s += join(transform<std::vector<std::string>>(children, [](auto &x) { return to_string(*x); }));
        s += ">";
        return s;
    }
};


struct template_break_node: template_node {
    template_break_node(ssize_t offset) noexcept : template_node(offset) {}

    std::string string() const noexcept override {
        return "<break>";
    }
};

struct template_continue_node: template_node {
    template_continue_node(ssize_t offset) noexcept : template_node(offset) {}

    std::string string() const noexcept override {
        return "<continue>";
    }
};

struct template_return_node: template_node {
    std::unique_ptr<expression_node> expression;

    template_return_node(ssize_t offset, std::unique_ptr<expression_node> expression) noexcept :
        template_node(offset), expression(std::move(expression)) {}

    std::string string() const noexcept override {
        return fmt::format("<return {}>", *expression);
    }
};

void template_parse_context::start_of_text_segment(int back_track) noexcept
{
    text_segment_start = text_it - back_track;
}

[[nodiscard]] bool template_parse_context::top_statement_is_do() const noexcept
{
    if (statement_stack.size() < 1) {
        return false;
    }

    auto ptr = dynamic_cast<template_do_node*>(statement_stack.back().get());
    return ptr != nullptr;
}

void template_parse_context::end_of_text_segment()
{
    if (text_segment_start) {
        if (text_it > *text_segment_start) {
            let text_segment_offset = std::distance(first, *text_segment_start);
            if (!append<template_string_node>(text_segment_offset, std::string(*text_segment_start, text_it))) {
                TTAURI_THROW(parse_error("Unexpected text segment."));
            }
        }

        text_segment_start = {};
    }
}

std::unique_ptr<template_node> template_parser(template_parse_context &context)
{
    enum class state_t {
        TextSegment,
        Escape,
        FoundHash,
        FoundDollar,
    };

    context.start_of_text_segment();
    state_t state = state_t::TextSegment;

    while (!context.atEOF()) {
        let offset = context.offset();

        switch (state) {
        case state_t::TextSegment:
            switch (*context) {
            case '#':
                context.end_of_text_segment();
                state = state_t::FoundHash;
                break;
            case '$':
                context.end_of_text_segment();
                state = state_t::FoundDollar;
                break;
            case '\\': // Skip the backslash.
                context.end_of_text_segment();
                state = state_t::Escape;
                break;
            default:
                // Consume character.
                ;
            }
            break;

        case state_t::Escape:
            switch (*context) {
            case '\n': // Skip over line-feed
                context.start_of_text_segment();
                state = state_t::TextSegment;
                break;
            case '\r': // Skip over carriage return and potential line-feed.
                state = state_t::Escape;
                break;
            default: // Add character to text.
                context.start_of_text_segment(-1);
                state = state_t::TextSegment;
            }
            break;

        case state_t::FoundHash:
            if (context.starts_with("end")) {
                context.advance_over("\n");

                if (!context.pop()) {
                    TTAURI_THROW(parse_error("Unexpected #end statement."));
                }

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("if ")) {
                context.push<template_if_node>(offset, context.parse_expression("\n"));

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("elif ")) {
                if (!context.found_elif(offset, context.parse_expression("\n"))) {
                    TTAURI_THROW(parse_error("Unexpected #elif statement."));
                }

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("else")) {
                context.advance_over("\n");

                if (!context.found_else(offset)) {
                    TTAURI_THROW(parse_error("Unexpected #else statement."));
                }

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("for ")) {
                auto name_expression = context.parse_expression("in");
                auto list_expression = context.parse_expression("\n");

                context.push<template_for_node>(offset, std::move(name_expression), std::move(list_expression));

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("while ")) {
                auto expression = context.parse_expression("\n");

                if (context.top_statement_is_do()) {
                    if (!context.found_while(offset, std::move(expression))) {
                        TTAURI_THROW(parse_error("Unexpected #while statement; missing #do."));
                    }

                    required_assert(context.pop());
                } else {
                    context.push<template_while_node>(offset, std::move(expression));
                }

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("do")) {
                context.advance_over("\n");

                context.push<template_do_node>(offset);

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("function ")) {
                context.push<template_function_node>(offset, context.parse_expression("\n"));

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("block ")) {
                context.push<template_block_node>(offset, context.parse_expression("\n"));

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("break")) {
                context.advance_over("\n");

                if (!context.append<template_break_node>(offset)) {
                    TTAURI_THROW(parse_error("Unexpected #break statement"));
                }

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("continue")) {
                context.advance_over("\n");

                if (!context.append<template_continue_node>(offset)) {
                    TTAURI_THROW(parse_error("Unexpected #continue statement"));
                }

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("return ")) {
                if (!context.append<template_return_node>(offset, context.parse_expression("\n"))) {
                    TTAURI_THROW(parse_error("Unexpected #return statement"));
                }

                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else if (context.starts_with_and_advance_over("include ")) {
                context.include(offset, context.parse_expression("\n"));
                context.start_of_text_segment();
                state = state_t::TextSegment;

            } else { // Add '#' and the current character to text.
                context.start_of_text_segment(-2);
                state = state_t::TextSegment;
            }
            break;

        case state_t::FoundDollar:
            switch (*context) {
            case '{': { // Place holder.
                ++context;
                auto expression = context.parse_expression("}");

                if (!context.starts_with("}")) {
                    TTAURI_THROW(parse_error("Missing '}' on placeholder."));
                }

                if (!context.append<template_placeholder_node>(offset, std::move(expression))) {
                    TTAURI_THROW(parse_error("Unexpected placeholder."));
                }

                context.start_of_text_segment();
                state = state_t::TextSegment;
                } break;

            default: // Add '$' and current character to text.
                context.start_of_text_segment(-1);
                state = state_t::TextSegment;
                break;
            }
            break;
        }
    }
    context.end_of_text_segment();

    if (context.statement_stack.size() < 1) {
        TTAURI_THROW(parse_error("Found to many #end statements."));
    } else if (context.statement_stack.size() > 1) {
        TTAURI_THROW(parse_error("Missing #end statement."));
    }

    return std::move(context.statement_stack.back());
}

}