// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/tokenizer.hpp"
#include <string>
#include <string_view>
#include <memory>

namespace TTauri {

struct expression {
    std::unique_ptr<expression> lhs;
    token_t op;
    std::unique_ptr<expression> rhs;

    expression(token_t op) : op(std::move(op)) {}
    expression(std::unique_ptr<expression> lhs, token_t op, std::unique_ptr<expression> rhs) :
        lhs(std::move(lhs)), op(std::move(op)), rhs(std::move(rhs)) {}
};



inline void annotate_expression(token_iterator token)
{
    while (*token != tokenizer_name_t::End) {
        // Translate name tokens.
        if (*token == tokenizer_name_t::Name) {
            if (token->value == "or") {
                token->name = tokenizer_name_t::Literal;
                token->value = "||";
            } else if (token->value == "and") {
                token->name = tokenizer_name_t::Literal;
                token->value = "&&";
            } else if (token->value == "true" || token->value == "false") {
                token->name = tokenizer_name_t::BooleanLiteral;
            }
        }

        // Set precedence and associativity.
        if (*token == tokenizer_name_t::Literal) {
            if (token->value == ".") {
                token->precedence = 2;
                token->is_binary = true;

            } else if (token->value == "!" || token->value == "~" || token->value == "-") {
                token->precedence = 3;
                token->is_binary = false;

            } else if (token->value == "**") {
                token->precedence = 4;
                token->is_binary = true;

            } else if (token->value == "*" || token->value == "/" || token->value == "%") {
                token->precedence = 5;
                token->is_binary = true;

            } else if (token->value == "+" || token->value == "-") {
                token->precedence = 6;
                token->is_binary = true;

            } else if (token->value == "<<" || token->value == ">>") {
                token->precedence = 7;
                token->is_binary = true;

            } else if (token->value == "<=>") {
                token->precedence = 8;
                token->is_binary = true;

            } else if (token->value == "<" || token->value == ">" || token->value == "<=" || token->value == ">=") {
                token->precedence = 9;
                token->is_binary = true;

            } else if (token->value == "==" || token->value == "!=") {
                token->precedence = 10;
                token->is_binary = true;

            } else if (token->value == "&") {
                token->precedence = 11;
                token->is_binary = true;

            } else if (token->value == "^") {
                token->precedence = 12;
                token->is_binary = true;

            } else if (token->value == "|") {
                token->precedence = 13;
                token->is_binary = true;

            } else if (token->value == "&&") {
                token->precedence = 14;
                token->is_binary = true;

            } else if (token->value == "||") {
                token->precedence = 15;
                token->is_binary = true;

            } else {
                TTAURI_THROW(parse_error("Unknown operator '{}'", token));
            }
        }
    }
}
inline expression parse_expression(token_iterator &token, token_t lhs, int precedence=0)
{
    annotate_expression(token);
    auto lookahead = *token;

    while (lookahead != tokenizer_name_t::End && lookahead.precedence >= precedence) {
        auto op = lookahead;

        token++;
        auto rhs = expression(*token++);

        lookahead = *token;
        while (lookahead.is_binary ?
            lookahead.precedence > op.precedence :
            lookahead.precedence == op.precedence
        ) {
            rhs = parse_expression(token, std::move(rhs), op.precedence);
            lookahead = *token;
        }
        lhs = expression(std::move(lhs), op, std::move(rhs));
    }
    return lhs;
}



/** Find the end of an expression.
 * This function will track nested brackets and strings, until the terminating_character is found.
 * @param first Iterator to the first character of the expression.
 * @param last Iterator to beyond the last character of the text.
 * @param terminating_character The character to find.
 * @return Iterator to the terminating character if found, or last.
 */
inline std::string_view::const_iterator find_end_of_expression(
    std::string_view::const_iterator first,
    std::string_view::const_iterator last,
    char terminating_character)
{
    std::string bracket_stack;
    char in_string = 0;
    bool in_escape = false;

    for (auto i = first; i != last; i++) {
        if (in_escape) {
            in_escape = false;

        } else if (in_string) {
            if (*i == in_string) {
                in_string = 0;
            } else if (*i == '\\') {
                in_escape = true;
            }

        } else {
            switch (*i) {
            case '"': in_string = '"'; break;
            case '\'': in_string = '\''; break;
            case '{': bracket_stack += '}'; break;
            case '[': bracket_stack += ']'; break;
            case '(': bracket_stack += ')'; break;
            case '\\': in_escape = true; break; // It is possible to escape any character, including the terminating_character.
            default:
                if (bracket_stack.size() > 0) {
                    if (*i == bracket_stack.back()) {
                        bracket_stack.pop_back();
                    }
                } else if (*i == terminating_character) {
                    return i;
                }
            }
        }
    }
    return last;
}

}
