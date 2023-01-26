// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "formula.hpp"
#include "formula_add_node.hpp"
#include "formula_arguments.hpp"
#include "formula_assign_node.hpp"
#include "formula_binary_operator_node.hpp"
#include "formula_bit_and_node.hpp"
#include "formula_bit_or_node.hpp"
#include "formula_bit_xor_node.hpp"
#include "formula_call_node.hpp"
#include "formula_decrement_node.hpp"
#include "formula_div_node.hpp"
#include "formula_eq_node.hpp"
#include "formula_filter_node.hpp"
#include "formula_ge_node.hpp"
#include "formula_gt_node.hpp"
#include "formula_increment_node.hpp"
#include "formula_index_node.hpp"
#include "formula_inplace_add_node.hpp"
#include "formula_inplace_and_node.hpp"
#include "formula_inplace_div_node.hpp"
#include "formula_inplace_mod_node.hpp"
#include "formula_inplace_mul_node.hpp"
#include "formula_inplace_or_node.hpp"
#include "formula_inplace_shl_node.hpp"
#include "formula_inplace_shr_node.hpp"
#include "formula_inplace_sub_node.hpp"
#include "formula_inplace_xor_node.hpp"
#include "formula_invert_node.hpp"
#include "formula_le_node.hpp"
#include "formula_literal_node.hpp"
#include "formula_logical_and_node.hpp"
#include "formula_logical_not_node.hpp"
#include "formula_logical_or_node.hpp"
#include "formula_lt_node.hpp"
#include "formula_map_literal_node.hpp"
#include "formula_member_node.hpp"
#include "formula_minus_node.hpp"
#include "formula_mod_node.hpp"
#include "formula_mul_node.hpp"
#include "formula_name_node.hpp"
#include "formula_ne_node.hpp"
#include "formula_node.hpp"
#include "formula_plus_node.hpp"
#include "formula_pow_node.hpp"
#include "formula_shl_node.hpp"
#include "formula_shr_node.hpp"
#include "formula_sub_node.hpp"
#include "formula_ternary_operator_node.hpp"
#include "formula_unary_operator_node.hpp"
#include "formula_vector_literal_node.hpp"
#include "../operator.hpp"
#include "../strings.hpp"
#include "../utility/module.hpp"
#include <format>
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <limits>

namespace hi::inline v1 {

static std::unique_ptr<formula_node>
parse_formula_1(formula_parse_context &context, std::unique_ptr<formula_node> lhs, uint8_t min_precedence);

[[nodiscard]] std::pair<uint8_t, bool> operator_precedence(token_t const &token, bool binary) noexcept
{
    if (token != tokenizer_name_t::Operator) {
        return {uint8_t{0}, false};
    } else {
        auto [precedence, left_to_right] = operator_precedence(token.value.data(), binary);
        return {static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() - precedence), left_to_right};
    }
}

static std::unique_ptr<formula_node> parse_operation_formula(
    formula_parse_context &context,
    std::unique_ptr<formula_node> lhs,
    token_t const &op,
    std::unique_ptr<formula_node> rhs)
{
    if (lhs) {
        // Binary operator
        switch (operator_to_int(op.value.data())) {
        case operator_to_int("."): return std::make_unique<formula_member_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("**"): return std::make_unique<formula_pow_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("*"): return std::make_unique<formula_mul_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("/"): return std::make_unique<formula_div_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("%"): return std::make_unique<formula_mod_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("+"): return std::make_unique<formula_add_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("-"): return std::make_unique<formula_sub_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("<<"): return std::make_unique<formula_shl_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int(">>"): return std::make_unique<formula_shr_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("<"): return std::make_unique<formula_lt_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int(">"): return std::make_unique<formula_gt_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("<="): return std::make_unique<formula_le_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int(">="): return std::make_unique<formula_ge_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("=="): return std::make_unique<formula_eq_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("!="): return std::make_unique<formula_ne_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("&"): return std::make_unique<formula_bit_and_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("^"): return std::make_unique<formula_bit_xor_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("|"): return std::make_unique<formula_bit_or_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("&&"):
            return std::make_unique<formula_logical_and_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("||"): return std::make_unique<formula_logical_or_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("?"):
            return std::make_unique<formula_ternary_operator_node>(op.location, std::move(lhs), *rhs);
        case operator_to_int("["): return std::make_unique<formula_index_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("("): return std::make_unique<formula_call_node>(op.location, std::move(lhs), *rhs);
        case operator_to_int("="): return std::make_unique<formula_assign_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("+="):
            return std::make_unique<formula_inplace_add_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("-="):
            return std::make_unique<formula_inplace_sub_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("*="):
            return std::make_unique<formula_inplace_mul_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("/="):
            return std::make_unique<formula_inplace_div_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("%="):
            return std::make_unique<formula_inplace_mod_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("<<="):
            return std::make_unique<formula_inplace_shl_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int(">>="):
            return std::make_unique<formula_inplace_shr_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("&="):
            return std::make_unique<formula_inplace_and_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("|="): return std::make_unique<formula_inplace_or_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("^="):
            return std::make_unique<formula_inplace_xor_node>(op.location, std::move(lhs), std::move(rhs));
        case operator_to_int("!"): return std::make_unique<formula_filter_node>(op.location, std::move(lhs), std::move(rhs));
        default: throw parse_error(std::format("{}: Unexpected binary operator {}.", op.location, op));
        }
    } else {
        // Unary operator
        switch (operator_to_int(op.value.data())) {
        case operator_to_int("+"): return std::make_unique<formula_plus_node>(op.location, std::move(rhs));
        case operator_to_int("-"): return std::make_unique<formula_minus_node>(op.location, std::move(rhs));
        case operator_to_int("~"): return std::make_unique<formula_invert_node>(op.location, std::move(rhs));
        case operator_to_int("!"): return std::make_unique<formula_logical_not_node>(op.location, std::move(rhs));
        case operator_to_int("++"): return std::make_unique<formula_increment_node>(op.location, std::move(rhs));
        case operator_to_int("--"): return std::make_unique<formula_decrement_node>(op.location, std::move(rhs));
        default: throw parse_error(std::format("{}: Unexpected unary operator {}.", op.location, op));
        }
    }
}

/** Parse a lhs or rhs part of an formula.
 * This should expect any off:
 *  - leaf node: literal
 *  - leaf node: name
 *  - vector literal: '[' ( parse_formula() ( ',' parse_formula() )* ','? )? ']'
 *  - map literal: '{' ( parse_formula() ':' parse_formula() ( ',' parse_formula() ':' parse_formula() )* ','? )? '}'
 *  - subformula:  '(' parse_formula() ')'
 *  - unary operator: op parse_formula()
 */
static std::unique_ptr<formula_node> parse_primary_formula(formula_parse_context &context)
{
    hilet &location = context->location;

    switch (context->name) {
    case tokenizer_name_t::IntegerLiteral:
        return std::make_unique<formula_literal_node>(location, datum{static_cast<long long>(*context++)});

    case tokenizer_name_t::FloatLiteral:
        return std::make_unique<formula_literal_node>(location, datum{static_cast<double>(*context++)});

    case tokenizer_name_t::StringLiteral:
        return std::make_unique<formula_literal_node>(location, datum{static_cast<std::string>(*context++)});

    case tokenizer_name_t::Name:
        if (*context == "true") {
            ++context;
            return std::make_unique<formula_literal_node>(location, datum{true});

        } else if (*context == "false") {
            ++context;
            return std::make_unique<formula_literal_node>(location, datum{false});

        } else if (*context == "null") {
            ++context;
            return std::make_unique<formula_literal_node>(location, datum{nullptr});

        } else if (*context == "undefined") {
            ++context;
            return std::make_unique<formula_literal_node>(location, datum{});

        } else {
            return std::make_unique<formula_name_node>(location, (context++)->value);
        }

    case tokenizer_name_t::Operator:
        if (*context == "(") {
            ++context;
            auto subformula = parse_formula(context);

            if ((*context == tokenizer_name_t::Operator) && (*context == ")")) {
                ++context;
            } else {
                throw parse_error(std::format("{}: Expected ')' token for function call got {}.", location, *context));
            }

            return subformula;

        } else if (*context == "[") {
            ++context;

            formula_node::formula_vector values;

            // ',' is between each formula, but a ']' may follow a ',' directly.
            while (!((*context == tokenizer_name_t::Operator) && (*context == "]"))) {
                values.push_back(parse_formula(context));

                if ((*context == tokenizer_name_t::Operator) && (*context == ",")) {
                    ++context;
                } else if ((*context == tokenizer_name_t::Operator) && (*context == "]")) {
                    ++context;
                    break;
                } else {
                    throw parse_error(std::format("{}: Expected ']' or ',' after a vector sub-formula. got {}", location, *context));
                }
            }

            return std::make_unique<formula_vector_literal_node>(location, std::move(values));

        } else if (*context == "{") {
            ++context;

            formula_node::formula_vector keys;
            formula_node::formula_vector values;

            // ',' is between each formula, but a ']' may follow a ',' directly.
            while (!((*context == tokenizer_name_t::Operator) && (*context == "}"))) {
                keys.push_back(parse_formula(context));

                if ((*context == tokenizer_name_t::Operator) && (*context == ":")) {
                    ++context;
                } else {
                    throw parse_error(std::format("{}: Expected ':' after a map key. got {}", location, *context));
                }

                values.push_back(parse_formula(context));

                if ((*context == tokenizer_name_t::Operator) && (*context == ",")) {
                    ++context;
                } else if ((*context == tokenizer_name_t::Operator) && (*context == "}")) {
                    ++context;
                    break;
                } else {
                    throw parse_error(std::format("{}: Expected ']' or ',' after a vector sub-formula. got {}.", location, *context));
                }
            }

            return std::make_unique<formula_map_literal_node>(location, std::move(keys), std::move(values));

        } else {
            hilet unary_op = *context;
            ++context;
            hilet[precedence, left_to_right] = operator_precedence(unary_op, false);
            auto subformula = parse_formula_1(context, parse_primary_formula(context), precedence);
            return parse_operation_formula(context, {}, unary_op, std::move(subformula));
        }

    default: throw parse_error(std::format("{}: Unexpected token in primary formula {}.", location, *context));
    }
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
static std::unique_ptr<formula_node> parse_index_formula(formula_parse_context &context)
{
    auto rhs = parse_formula(context);

    if ((*context == tokenizer_name_t::Operator) && (*context == "]")) {
        ++context;
    } else {
        throw parse_error(std::format("{}: Expected ']' token at end of indexing operator got {}.", context->location, *context));
    }
    return rhs;
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
static std::unique_ptr<formula_node> parse_ternary_argument_formula(formula_parse_context &context)
{
    auto rhs_true = parse_formula(context);

    if ((*context == tokenizer_name_t::Operator) && (*context == ":")) {
        ++context;
    } else {
        throw parse_error(std::format("{}: Expected ':' token in ternary formula {}.", context->location, *context));
    }

    auto rhs_false = parse_formula(context);

    return std::make_unique<formula_arguments>(context->location, std::move(rhs_true), std::move(rhs_false));
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
static std::unique_ptr<formula_node> parse_call_argument_formula(formula_parse_context &context)
{
    formula_node::formula_vector args;

    if ((*context == tokenizer_name_t::Operator) && (*context == ")")) {
        ++context;

    } else
        while (true) {
            args.push_back(parse_formula(context));

            if ((*context == tokenizer_name_t::Operator) && (*context == ",")) {
                ++context;
                continue;

            } else if ((*context == tokenizer_name_t::Operator) && (*context == ")")) {
                ++context;
                break;

            } else {
                throw parse_error(std::format("{}: Expected ',' or ')' After a function argument {}.", context->location, *context));
            }
        }

    return std::make_unique<formula_arguments>(context->location, std::move(args));
}

static bool parse_formula_is_at_end(formula_parse_context &context)
{
    if (*context == tokenizer_name_t::End) {
        return true;
    }

    if (*context != tokenizer_name_t::Operator) {
        throw parse_error(std::format("{}: Expecting an operator token got {}.", context->location, *context));
    }

    return *context == ")" || *context == "}" || *context == "]" || *context == ":" || *context == ",";
}

/** Parse an formula.
 * https://en.wikipedia.org/wiki/Operator-precedence_parser
 * Parses an formula until EOF, ')', '}', ']', ':', ','
 */
static std::unique_ptr<formula_node>
parse_formula_1(formula_parse_context &context, std::unique_ptr<formula_node> lhs, uint8_t min_precedence)
{
    token_t lookahead;
    uint8_t lookahead_precedence;
    bool lookahead_left_to_right;

    std::tie(lookahead_precedence, lookahead_left_to_right) = operator_precedence(lookahead = *context, true);
    if (parse_formula_is_at_end(context)) {
        return lhs;
    }

    while (lookahead_precedence >= min_precedence) {
        hilet op = lookahead;
        hilet op_precedence = lookahead_precedence;
        ++context;

        std::unique_ptr<formula_node> rhs;
        if (op == tokenizer_name_t::Operator && op == "[") {
            rhs = parse_index_formula(context);
        } else if (op == tokenizer_name_t::Operator && op == "(") {
            rhs = parse_call_argument_formula(context);
        } else if (op == tokenizer_name_t::Operator && op == "?") {
            rhs = parse_ternary_argument_formula(context);
        } else {
            rhs = parse_primary_formula(context);
        }

        std::tie(lookahead_precedence, lookahead_left_to_right) = operator_precedence(lookahead = *context, true);
        if (parse_formula_is_at_end(context)) {
            return parse_operation_formula(context, std::move(lhs), op, std::move(rhs));
        }

        while ((lookahead_left_to_right == true && lookahead_precedence > op_precedence) ||
               (lookahead_left_to_right == false && lookahead_precedence == op_precedence)) {
            rhs = parse_formula_1(context, std::move(rhs), lookahead_precedence);

            std::tie(lookahead_precedence, lookahead_left_to_right) = operator_precedence(lookahead = *context, true);
            if (parse_formula_is_at_end(context)) {
                return parse_operation_formula(context, std::move(lhs), op, std::move(rhs));
            }
        }
        lhs = parse_operation_formula(context, std::move(lhs), op, std::move(rhs));
    }
    return lhs;
}

std::unique_ptr<formula_node> parse_formula(formula_parse_context &context)
{
    return parse_formula_1(context, parse_primary_formula(context), 0);
}

std::string_view::const_iterator find_end_of_formula(
    std::string_view::const_iterator first,
    std::string_view::const_iterator last,
    std::string_view terminating_string)
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

                } else if (std::string_view{i, last}.starts_with(terminating_string)) {
                    return i;
                }
            }
        }
    }
    return last;
}

} // namespace hi::inline v1
