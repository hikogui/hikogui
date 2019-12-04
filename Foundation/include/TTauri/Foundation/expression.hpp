// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/tokenizer.hpp"
#include "TTauri/Foundation/datum.hpp"
#include "TTauri/Foundation/Location.hpp"
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

namespace TTauri {

struct expression_evaluation_context {
    using scope = std::unordered_map<std::string, datum>;
    using stack = std::vector<scope>;

    stack local_stack;
    scope globals;

    evaluation_context() {
        push();
    }

    void push() {
        local_stack.push_back();
    }

    void pop() {
        required_assert(local_stack.size() > 1);
        local_stack.pop_back();
    }

    force_inline scope const &locals() const {
        axiom_assert(local_stack.size() >= 1);
        return local_stack.back();
    }

    force_inline scope& locals() {
        axiom_assert(local_stack.size() >= 1);
        return local_stack.back();
    }

    datum const &get(std::string_view name) const {
        let i = locals().find(name);
        if (i != locals().end()) {
            return *i;
        }

        let j = globals.find(name);
        if (j != globals.end()) {
            return *j;
        }

        TTAURI_THROW(key_error("Could not find {} in local or global scope.", name));
    }

    void set_local(std::string_view name, datum &value) {
        locals()[name] = value;
    }
    
    void set_global(std::string_view name, datum& value) {
        globals[name] = value;
    }
};



struct expression {
    Location location;
    expression(Location const &location) : location(location) {}
    virtual datum evaluate(expression_evaluation_context&context) const = 0;
};

struct expression_pair : expression {
    std::unique_ptr<expression> arg1;
    std::unique_ptr<expression> arg2;
    expression_ternary_operator(Location const& location, std::unique_ptr<expression> lhs, std::unique_ptr<expression> arg1, std::unique_ptr<expression> arg2) :
        expression(location), lhs(std::move(lhs)), arg1(std::move(arg1)), arg2(std::move(arg2))) {}

    virtual datum evaluate(expression_evaluation_context& context) const { return {} };
};

struct expression_literal : expression {
    datum value;
    expression_literal(Location const& location, datum const &value) : expression(location), value(value) {}
    datum evaluate(expression_evaluation_context& context) const override { return value; }
};

struct expression_name : expression {
    std::string name;
    expression_name(Location const& location, std::string_view name) : expression(location), name(name) {}
    datum evaluate(expression_evaluation_context& context) const override { return context.get(name); }
};

struct expression_call : expression {
    std::function<datum(expression_evaluation_context&,datum::vector)> func;
    std::vector<std::unique_ptr<expression>> args;

    expression_name(Location const& location, std::function<datum(expression_evaluation_context&,datum::vector)> func, std::vector<std::unique_ptr<expression>> args) :
        expression(location), func(std::move(func)), args(std::move(args)) {}

    datum evaluate(expression_evaluation_context& context) const override {
        let args_ = transform<datum::vector>(args, [](let &x) {
            return x.evaluate(context);
        });

        return func(context, args_);
    }
};

struct expression_unary_operator : expression {
    std::unique_ptr<expression> rhs;
    expression_unary_operator(Location const& location, std::unique_ptr<expression> rhs) :
        expression(location), rhs(std::move(rhs))) {}
};

struct expression_binary_operator : expression {
    std::unique_ptr<expression> lhs;
    std::unique_ptr<expression> rhs;
    expression_binary_operator(Location const& location, std::unique_ptr<expression> lhs, std::unique_ptr<expression> rhs) :
        expression(location), lhs(std::move(lhs)), rhs(std::move(rhs))) {}
};

struct expression_ternary_operator : expression {
    std::unique_ptr<expression> lhs;
    std::unique_ptr<expression> rhs_true;
    std::unique_ptr<expression> rhs_false;
    expression_ternary_operator(Location const& location, std::unique_ptr<expression> pair) :
        expression(location), lhs(std::move(lhs))
    {
        expression_pair *pair_ = dynamic_cast<expression_pair *>(pair.get());
        required_assert(pair_ != nullptr);
        rhs_true = std::move(pair_->arg1);
        rhs_false = std::move(pair_->arg2);
    }

    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) ? rhs_true.evaluate(context) : rhs_false.evaluate(context); }
};

struct expression_plus : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return +rhs.evaluate(context); }
};
struct expression_minus : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return -rhs.evaluate(context); }
};
struct expression_invert : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return ~rhs.evaluate(context); }
};
struct expression_logical_not : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return !rhs.evaluate(context); }
};

struct expression_add : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) + rhs.evaluate(context); }
};
struct expression_sub : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) - rhs.evaluate(context); }
};
struct expression_mul : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) * rhs.evaluate(context); }
};
struct expression_div : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) / rhs.evaluate(context); }
};
struct expression_mod : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) % rhs.evaluate(context); }
};
struct expression_pow : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { not_implemented; }
};
struct expression_logical_and : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) && rhs.evaluate(context); }
};
struct expression_logical_or : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) || rhs.evaluate(context); }
};
struct expression_bit_and : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) & rhs.evaluate(context); }
};
struct expression_bit_or : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) | rhs.evaluate(context); }
};
struct expression_bit_xor : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) ^ rhs.evaluate(context); }
};
struct expression_shl : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) << rhs.evaluate(context); }
};
struct expression_shr : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) >> rhs.evaluate(context); }
};
struct expression_eq : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) == rhs.evaluate(context); }
};
struct expression_ne : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) != rhs.evaluate(context); }
};
struct expression_lt : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) < rhs.evaluate(context); }
};
struct expression_gt : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) > rhs.evaluate(context); }
};
struct expression_le : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) <= rhs.evaluate(context); }
};
struct expression_ge : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context) >= rhs.evaluate(context); }
};
struct expression_index : expression_binary_operator {
    datum evaluate(expression_evaluation_context& context) const override { return lhs.evaluate(context)[rhs.evaluate(context)]; }
};

constexpr uint32_t operator_to_int(char const *str) noexcept {
    uint32_t v = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        v <<= 8;
        v |= str[i];
    }
}


/** Operator Precedence according to C++.
 */
constexpr uint8_t binary_operator_precedence(token_t const &token) noexcept {
    if (*token != tokenizer_name_t::Literal) {
        return std::numeric_limits<uint8_t>::max();
    }

    switch (operator_to_int(token->value.data()) {
    case operator_to_int("::"): return 1;
    case operator_to_int("("): return 2;
    case operator_to_int("["): return 2;
    case operator_to_int("."): return 2;
    case operator_to_int("->"): return 2;
    case operator_to_int(".*"): return 4;
    case operator_to_int("->*"): return 4;
    case operator_to_int("**"): return 4;
    case operator_to_int("*"): return 5;
    case operator_to_int("/"): return 5;
    case operator_to_int("%"): return 5;
    case operator_to_int("+"): return 6;
    case operator_to_int("-"): return 6;
    case operator_to_int("<<"): return 7;
    case operator_to_int(">>"): return 7;
    case operator_to_int("<=>"): return 8;
    case operator_to_int("<"): return 9;
    case operator_to_int(">"): return 9;
    case operator_to_int("<="): return 9;
    case operator_to_int(">="): return 9;
    case operator_to_int("=="): return 10;
    case operator_to_int("!="): return 10;
    case operator_to_int("&"): return 11;
    case operator_to_int("^"): return 12;
    case operator_to_int("|"): return 13;
    case operator_to_int("&&"): return 14;
    case operator_to_int("||"): return 15;
    case operator_to_int("?"): return 16;
    case operator_to_int("="): return 16;
    case operator_to_int("+="): return 16;
    case operator_to_int("-="): return 16;
    case operator_to_int("*="): return 16;
    case operator_to_int("/="): return 16;
    case operator_to_int("%="): return 16;
    case operator_to_int("<<="): return 16;
    case operator_to_int(">>="): return 16;
    case operator_to_int("&="): return 16;
    case operator_to_int("^="): return 16;
    case operator_to_int("|="): return 16;
    case operator_to_int(","): return 17;
    case operator_to_int("]"): return 17;
    case operator_to_int(")"): return 17;
    default: std::numeric_limits<uint8_t>::max();
    }
}

struct expression_parse_context {
    using const_iterator = typename std::vector<token_t>::const_iterator;

    std::vector<token_t> tokens;
    const_iterator index;
    const_iterator end;

    [[nodiscard]] token_t const &peek() const noexcept {
        return *index;
    }

    token_t const &advance() noexcept {
        required_assert(index != end);
        required_assert(*index != tokenizer_name_t::End);
        return *index++;
    }

    std::function<datum(expression_evaluation_context&,datum::vector)> get_function(std::string_view name) const {

    }
};

inline std::unique_ptr<expression> parse_binary_operation_expression(
    expression_parse_context& constext, std::unique_ptr<expression> lhs, token_t const &op, std::unique_ptr<expression> rhs
) {
    switch (operator_to_int(op.value)) {
    case operator_to_int("**"): return make_unique<expression_pow>(op.location, lhs, rhs);
    case operator_to_int("*"): return make_unique<expression_mul>(op.location, lhs, rhs);
    case operator_to_int("/"): return make_unique<expression_div>(op.location, lhs, rhs);
    case operator_to_int("%"): return make_unique<expression_mod>(op.location, lhs, rhs);
    case operator_to_int("+"): return make_unique<expression_add>(op.location, lhs, rhs);
    case operator_to_int("-"): return make_unique<expression_sub>(op.location, lhs, rhs);
    case operator_to_int("<<"): return make_unique<expression_shl>(op.location, lhs, rhs);
    case operator_to_int(">>"): return make_unique<expression_shr>(op.location, lhs, rhs);
    case operator_to_int("<"): return make_unique<expression_lt>(op.location, lhs, rhs);
    case operator_to_int(">"): return make_unique<expression_gt>(op.location, lhs, rhs);
    case operator_to_int("<="): return make_unique<expression_le>(op.location, lhs, rhs);
    case operator_to_int(">="): return make_unique<expression_ge>(op.location, lhs, rhs);
    case operator_to_int("=="): return make_unique<expression_eq>(op.location, lhs, rhs);
    case operator_to_int("!="): return make_unique<expression_ne>(op.location, lhs, rhs);
    case operator_to_int("&"): return make_unique<expression_bit_and>(op.location, lhs, rhs);
    case operator_to_int("^"): return make_unique<expression_bit_xor>(op.location, lhs, rhs);
    case operator_to_int("|"): return make_unique<expression_bit_or>(op.location, lhs, rhs);
    case operator_to_int("&&"): return make_unique<expression_logical_and>(op.location, lhs, rhs);
    case operator_to_int("||"): return make_unique<expression_logical_or>(op.location, lhs, rhs);
    case operator_to_int("?"): return make_unique<expression_ternary_operator>(op.location, lhs, rhs);
    default: TTAURI_THROW(parse_error("Unexpected binary operator {}", op));
    }
}

/** Parse an expression.
 * Parses an expression until EOF, ')', ',', '}'
 */
std::unique_ptr<expression> parse_expression(expression_parse_context& context);

/** Parse a lhs or rhs part of an expression.
 * This should expect any off:
 *  - leaf node: literal
 *  - leaf node: name
 *  - subexpression:  '(' parse_expression() ')'
 *  - unary operator: op parse_expression()
 *  - function call: name '(' ( parse_expression() ( ',' parse_expression() )* )? ')'
 */
inline std::unique_ptr<expression> parse_primary_expression(expression_parse_context& context)
{
    auto &lookahead = context.peek();

    switch (lookahead.name) {
    case tokenizer_name_t::IntegerLiteral:
        context.advance();
        return std::make_unique<expression_literal>(lookahead.location, static_cast<long long>(lookahead));

    case tokenizer_name_t::FloatLiteral:
        context.advance();
        return std::make_unique<expression_literal>(lookahead.location, static_cast<double>(lookahead));

    case tokenizer_name_t::StringLiteral:
        context.advance();
        return std::make_unique<expression_literal>(lookahead.location, static_cast<std::string>(lookahead));

    case tokenizer_name_t::Name:
        if (lookahead.value == "true") {
            context.advance();
            return std::make_unique<expression_literal>(lookahead.location, true)

        } else if (lookahead == "false") {
            context.advance();
            return std::make_unique<expression_literal>(lookahead.location, false)

        } else if (lookahead == "null") {
            context.advance();
            return std::make_unique<expression_literal>(lookahead.location, datum::null)

        } else if (lookahead == "undefined") {
            context.advance();
            return std::make_unique<expression_literal>(lookahead.location, datum::undefined)

        } else if (auto func = context.get_function(lookahead.name)) {
            context.advance();
            lookahead = context.peek();

            std::vector<std::unique_ptr<expression>> args;

            if ((lookahead == tokenizer_name_t::Literal) && (lookahead == "(")) {
                context.advance();
                lookahead = context.peek();
            } else {
                TTAURI_THROW(parse_error("Expected '(' token for function call got {}", lookahead);
            }

            if ((lookahead == tokenizer_name_t::Literal) && (lookahead == ")")) {
                context.advance();

            } else {
                while (true) {
                    args.push_back(parse_expression(context));

                    if ((lookahead == tokenizer_name_t::Literal) && (lookahead == ",")) {
                        context.advance();
                        lookahead = context.peek();
                    } else {
                        break;
                    }
                }

                if ((lookahead == tokenizer_name_t::Literal) && (lookahead == ")")) {
                    context.advance();
                } else {
                    TTAURI_THROW(parse_error("Expected ')' token for function call got {}", lookahead);
                }
            }

            return std::make_unique<expression_call>(lookahead.location, std::move(func), std::move(args));

        } else {
            context.advance();
            return std::make_unique<expression_name>(lookahead.location, lookahead.name);
        }

    case tokenizer_name_t::Literal:
        if (lookahead == "+") {
            context.advance();
            return std::make_unique<expression_plus>(lookahead.location, parse_primary_expression(context));

        } else if (lookahead == "-") {
            context.advance();
            return std::make_unique<expression_minus>(lookahead.location, parse_primary_expression(context));

        } else if (lookahead == "~") {
            context.advance();
            return std::make_unique<expression_invert>(lookahead.location, parse_primary_expression(context));

        } else if (lookahead == "!") {
            context.advance();
            return std::make_unique<expression_logical_not>(lookahead.location, parse_primary_expression(context));

        } else if (lookahead == "(") {
            context.advance();
            auto subexpression = parse_expression(context);

            if ((lookahead == tokenizer_name_t::Literal) && (lookahead == ")")) {
                context.advance();
            } else {
                TTAURI_THROW(parse_error("Expected ')' token for function call got {}", lookahead);
            }

            return std::move(subexpression);

        } else {
            TTAURI_THROW(parse_error("Unexpected operator in primary expression {}", lookahead);
        }

    default:
        TTAURI_THROW(parse_error("Unexpected token in primary expression {}", lookahead);
    }
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
inline std::unique_ptr<expression> parse_index_expression(expression_parse_context& context)
{
    auto rhs = parse_expression(context);

    auto end_bracket = context.peek();
    if ((end_bracket == tokenizer_name_t::Literal) && (end_bracket == "]")) {
        context.advance();
    } else {
        TTAURI_THROW(parse_error("Expected ']' token at end of indexing operator got {}", lookahead);
    }
    return rhs;
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
inline std::unique_ptr<expression> parse_pair_expression(expression_parse_context& context)
{
    auto rhs_true = parse_expression(context);

    auto end_bracket = context.peek();
    if ((end_bracket == tokenizer_name_t::Literal) && (end_bracket == ":")) {
        context.advance();
    } else {
        TTAURI_THROW(parse_error("Expected ':' token in ternary expression {}", lookahead);
    }

    auto rhs_false = parse_expression(context);

    return make_unique<expression_pair>(std::move(rhs_true), std::move(rhs_false));
}


inline parse_expression_is_at_end(token_t const& lookahead)
{
    if (lookahead == tokenizer_name_t::End) {
        return true;
    }

    if (lookahead != tokenizer_name_t::Literal) {
        TTAURI_THROW(parse_error("Expecting an operator token got {}", lookahead);
    }

    return
        lookahead == ")" ||
        lookahead == "}" ||
        lookahead == "]" ||
        lookahead == ":" ||
        lookahead == ","
}


/** Parse an expression.
 * Parses an expression until EOF, ')', '}', ']', ':', ','
 */
inline std::unique_ptr<expression> parse_expression_1(expression_parse_context &context, std::unique_ptr<expression> lhs, uint8_t precedence)
{
    auto lookahead = context.peek();

    while (!parse_expression_is_at_end(lookahead) && binary_operator_precedence(lookahead.precedence) <= precedence) {
        let op = lookahead;
        let op_precedence = binary_operator_precedence(op.precedence)
        context.advance();

        std::unique_ptr<expression> rhs;
        if (op == tokenizer_name_t::Literal && op == "[") {
            rhs = parse_index_expression(context)
        } else if (op == tokenizer_name_t::Literal && op == "?") {
            rhs = parse_pair_expression(context)
        } else {
            rhs = parse_primary_expression(context);
        }
            
        lookahead = context.peek();
        while (!parse_expression_is_at_end(lookahead) && binary_operator_precedence(lookahead.precedence) < op_precedence) {
            rhs = parse_expression_1(context, std::move(rhs), op_precedence);
            lookahead = context.peek();
        }
        lhs = parse_binary_operation_expression(context, std::move(lhs), op, std::move(rhs));
    }
    return lhs;
}

inline std::unique_ptr<expression> parse_expression(expression_parse_context &context)
{
    return parse_expression_1(parse_primary_expression(context), std::numeric_limits<uint8_t>::max());
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
