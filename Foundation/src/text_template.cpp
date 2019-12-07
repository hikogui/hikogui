// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/text_template.hpp"

namespace TTauri {

struct text_template_if: text_template {
    std::vector<statement_vector> children_groups;
    std::vector<std::unique_ptr<expression>> expressions;

    text_template_if(std::unique_ptr<expression> expression) noexcept {
        expressions.push_back(std::move(expression));
        children_groups.push_back();
    }

    bool found_elif(std::unique_ptr<expression> x) noexcept override {
        if (children_groups.size() != expressions.size()) {
            return false;
        }

        expressions.push_back(std::move(x));
        children_groups.push_back();
        return true;
    }

    bool found_else() noexcept override {
        if (children_groups.size() != expressions.size()) {
            return false;
        }

        children_groups.push_back();
        return true;
    }

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<text_template> x) noexcept override {
        children_groups.back().push_back(std::move(x));
        return true;
    }
};

struct text_template_while: text_template {
    statement_vector children;
    std::unique_ptr<expression> expression;

    text_template_while(std::unique_ptr<expression> expression) noexcept :
        expression(std::move(expression)) {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<text_template> x) noexcept override {
        children.push_back(std::move(x));
        return true;
    }

    bool found_while(std::unique_ptr<expression> x) noexcept override {
        if (expression) {
            return false;
        } else {
            expression = std::move(x);
            return true;
        }
    }
};

struct text_template_do: text_template {
    statement_vector children;
    std::unique_ptr<expression> expression;

    text_template_do() noexcept {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<text_template> x) noexcept override {
        if (expression) {
            return false;
        } else {
            children.push_back(std::move(x));
            return true;
        }
    }
};

struct text_template_for: text_template {
    std::unique_ptr<expression> name_expression;
    std::unique_ptr<expression> list_expression;
    statement_vector children;

    text_template_for(std::unique_ptr<expression> name_expression, std::unique_ptr<expression> list_expression) noexcept :
        name_expression(std::move(name_expression)), list_expression(std::move(list_expression)) {}

    /** Append a template-piece to the current template.
    */
    bool append(std::unique_ptr<text_template> x) noexcept override {
        children.push_back(std::move(x));
        return true;
    }
};

struct text_template_break: text_template {
    text_template_break() noexcept : text_template() {}
};

struct text_template_continue: text_template {
    text_template_continue() noexcept : text_template() {}
};

struct text_template_return: text_template {
    std::unique_ptr<expression> expression;

    text_template_continue(std::unique_ptr<expression> expression) noexcept : text_template(), expression(expression) {}
};


std::unique_ptr<text_template> text_template_parser(text_template_parse_context &context)
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
            }

        case state_t::Escape:
            switch (*context) {
            case '\n': // Skip over line-feed
                context.start_of_text_segment();
                state = state_t::Idle;
                break;
            case '\r': // Skip over carriage return and potential line-feed.
                state = state_t::Escape;
                break;
            default: // Add character to text.
                context.start_of_text_segment(-1);
                state = state_t::Idle;
            }
            break;

        case state_t::FoundHash:
            if (starts_with(context.starts_with_and_advance("end")) {
                context.advance_over("\n");

                if (!context.pop()) {
                    TTAURI_THROW(parse_error("Unexpected #end statement."));
                }

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("if ")) {
                auto expression = context.parse_expression("\n");

                context.push<text_template_if>(expression);

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("elif ")) {
                auto expression = context.parse_expression("\n");

                if (!context.found_elif(expression)) {
                    TTAURI_THROW(parse_error("Unexpected #elif statement."));
                }

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("else")) {
                context.advance_over("\n");

                if (!context.found_else()) {
                    TTAURI_THROW(parse_error("Unexpected #else statement."));
                }

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("for ")) {
                auto name_expression = context.parse_expression("in");
                auto list_expression = context.parse_expression("\n");

                context.push<text_template_for>(name_expression, list_expression);

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("while ")) {
                auto expression = context.parse_expression("\n");

                if (context.found_while(expression)) {
                    context.pop();
                } else {
                    context.push<text_template_while>(expression);
                }

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("do")) {
                context.advance_over("\n");

                context.push<text_template_do>();

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("function ")) {
                auto expression = context.parse_expression("\n");

                context.push<text_template_function>(expression);

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("block ")) {
                auto expression = context.parse_expression("\n");

                context.push<text_template_block>(expression);

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("break")) {
                context.advance_over("\n");

                if (!context.append<text_template_break>()) {
                    TTAURI_THROW(parse_error("Unexpected #break statement"));
                }

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("continue")) {
                context.advance_over("\n");

                if (!context.append<text_template_continue>()) {
                    TTAURI_THROW(parse_error("Unexpected #continue statement"));
                }

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("return ")) {
                auto expression = context.parse_expression("\n");

                if (!context.append<text_template_return>(context)) {
                    TTAURI_THROW(parse_error("Unexpected #return statement"));
                }

                context.start_of_text_segment();
                state = state_t::Idle;

            } else if (context.starts_with_and_advance("include ")) {
                auto expression = context.parse_expression("\n");

                context.include(expression);

                context.start_of_text_segment();
                state = state_t::Idle;

            } else { // Add '#' and the current character to text.
                context.start_of_text_segment(-2);
                state = state_t::Idle;
            }

        case state_t::FoundDollar:
            switch (*context) {
            case '{': // Place holder.
                context++;
                auto expression = context.parse_expression("}");

                if (!context.starts_with("}"))
                    TTAURI_THROW(parse_error("Missing '}' on placeholder."));
                }
                context++;

                context.add<text_template_placeholder>(expression);

                context.index = expression_end;
                context.start_of_text_segment();
                state = state_t::Idle;
                break;

            default: // Add '$' and current character to text.
                context.start_of_text_segment(-2);
                state = state_t::Idle;
                break;
            }
            break;
        }
    }
    context.end_of_text_segment();

}

}