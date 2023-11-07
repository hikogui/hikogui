// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <string>
#include <string_view>
#include <memory>

export module hikogui_formula_formula_parser;
import hikogui_formula_formula_add_node;
import hikogui_formula_formula_arguments;
import hikogui_formula_formula_assign_node;
import hikogui_formula_formula_binary_operator_node;
import hikogui_formula_formula_bit_and_node;
import hikogui_formula_formula_bit_or_node;
import hikogui_formula_formula_bit_xor_node;
import hikogui_formula_formula_call_node;
import hikogui_formula_formula_decrement_node;
import hikogui_formula_formula_div_node;
import hikogui_formula_formula_eq_node;
import hikogui_formula_formula_evaluation_context;
import hikogui_formula_formula_filter_node;
import hikogui_formula_formula_ge_node;
import hikogui_formula_formula_gt_node;
import hikogui_formula_formula_increment_node;
import hikogui_formula_formula_index_node;
import hikogui_formula_formula_inplace_add_node;
import hikogui_formula_formula_inplace_and;
import hikogui_formula_formula_inplace_div_node;
import hikogui_formula_formula_inplace_mod_node;
import hikogui_formula_formula_inplace_mul_node;
import hikogui_formula_formula_inplace_or_node;
import hikogui_formula_formula_inplace_shl_node;
import hikogui_formula_formula_inplace_shr_node;
import hikogui_formula_formula_inplace_sub_node;
import hikogui_formula_formula_inplace_xor_node;
import hikogui_formula_formula_invert_node;
import hikogui_formula_formula_le_node;
import hikogui_formula_formula_literal_node;
import hikogui_formula_formula_logical_and_node;
import hikogui_formula_formula_logical_not_node;
import hikogui_formula_formula_logical_or_node;
import hikogui_formula_formula_lt_node;
import hikogui_formula_formula_map_literal_node;
import hikogui_formula_formula_member_node;
import hikogui_formula_formula_minus_node;
import hikogui_formula_formula_mod_node;
import hikogui_formula_formula_mul_node;
import hikogui_formula_formula_name_node;
import hikogui_formula_formula_ne_node;
import hikogui_formula_formula_node;
import hikogui_formula_formula_plus_node;
import hikogui_formula_formula_post_process_context;
import hikogui_formula_formula_pow_node;
import hikogui_formula_formula_shl_node;
import hikogui_formula_formula_shr_node;
import hikogui_formula_formula_sub_node;
import hikogui_formula_formula_ternary_operator_node;
import hikogui_formula_formula_unary_operator_node;
import hikogui_formula_formula_vector_literal_node;

export namespace hi { inline namespace v1 {

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
std::unique_ptr<formula_node> parse_formula_1(It& it, ItEnd last, std::unique_ptr<formula_node> lhs, uint8_t min_precedence);

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
std::unique_ptr<formula_node> parse_formula(It& it, ItEnd last);

[[nodiscard]] constexpr std::pair<uint8_t, bool> operator_precedence(token const& op, bool binary) noexcept
{
    if (op != token::other) {
        return {uint8_t{0}, false};
    } else {
        hilet[precedence, left_to_right] = operator_precedence(static_cast<std::string>(op), binary);
        return {static_cast<uint8_t>(std::numeric_limits<uint8_t>::max() - precedence), left_to_right};
    }
}

[[nodiscard]] std::unique_ptr<formula_node>
parse_operation_formula(std::unique_ptr<formula_node> lhs, token const& op, std::unique_ptr<formula_node> rhs)
{
    if (lhs) {
        // Binary operator
        switch (operator_to_int(static_cast<std::string>(op))) {
        case operator_to_int("."):
            return std::make_unique<formula_member_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("**"):
            return std::make_unique<formula_pow_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("*"):
            return std::make_unique<formula_mul_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("/"):
            return std::make_unique<formula_div_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("%"):
            return std::make_unique<formula_mod_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("+"):
            return std::make_unique<formula_add_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("-"):
            return std::make_unique<formula_sub_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("<<"):
            return std::make_unique<formula_shl_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int(">>"):
            return std::make_unique<formula_shr_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("<"):
            return std::make_unique<formula_lt_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int(">"):
            return std::make_unique<formula_gt_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("<="):
            return std::make_unique<formula_le_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int(">="):
            return std::make_unique<formula_ge_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("=="):
            return std::make_unique<formula_eq_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("!="):
            return std::make_unique<formula_ne_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("&"):
            return std::make_unique<formula_bit_and_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("^"):
            return std::make_unique<formula_bit_xor_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("|"):
            return std::make_unique<formula_bit_or_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("&&"):
            return std::make_unique<formula_logical_and_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("||"):
            return std::make_unique<formula_logical_or_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("?"):
            return std::make_unique<formula_ternary_operator_node>(op.line_nr, op.column_nr, std::move(lhs), *rhs);
        case operator_to_int("["):
            return std::make_unique<formula_index_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("("):
            return std::make_unique<formula_call_node>(op.line_nr, op.column_nr, std::move(lhs), *rhs);
        case operator_to_int("="):
            return std::make_unique<formula_assign_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("+="):
            return std::make_unique<formula_inplace_add_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("-="):
            return std::make_unique<formula_inplace_sub_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("*="):
            return std::make_unique<formula_inplace_mul_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("/="):
            return std::make_unique<formula_inplace_div_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("%="):
            return std::make_unique<formula_inplace_mod_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("<<="):
            return std::make_unique<formula_inplace_shl_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int(">>="):
            return std::make_unique<formula_inplace_shr_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("&="):
            return std::make_unique<formula_inplace_and_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("|="):
            return std::make_unique<formula_inplace_or_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("^="):
            return std::make_unique<formula_inplace_xor_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        case operator_to_int("!"):
            return std::make_unique<formula_filter_node>(op.line_nr, op.column_nr, std::move(lhs), std::move(rhs));
        default:
            throw parse_error(std::format("{}: Unexpected binary operator {}.", op.line_nr, op.column_nr, op));
        }
    } else {
        // Unary operator
        switch (operator_to_int(static_cast<std::string>(op))) {
        case operator_to_int("+"):
            return std::make_unique<formula_plus_node>(op.line_nr, op.column_nr, std::move(rhs));
        case operator_to_int("-"):
            return std::make_unique<formula_minus_node>(op.line_nr, op.column_nr, std::move(rhs));
        case operator_to_int("~"):
            return std::make_unique<formula_invert_node>(op.line_nr, op.column_nr, std::move(rhs));
        case operator_to_int("!"):
            return std::make_unique<formula_logical_not_node>(op.line_nr, op.column_nr, std::move(rhs));
        case operator_to_int("++"):
            return std::make_unique<formula_increment_node>(op.line_nr, op.column_nr, std::move(rhs));
        case operator_to_int("--"):
            return std::make_unique<formula_decrement_node>(op.line_nr, op.column_nr, std::move(rhs));
        default:
            throw parse_error(std::format("{}:{}: Unexpected unary operator {}.", op.line_nr, op.column_nr, op));
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
template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] std::unique_ptr<formula_node> parse_primary_formula(It& it, ItEnd last)
{
    hi_assert(it != last);
    hilet line_nr = it->line_nr;
    hilet column_nr = it->column_nr;

    if (*it == token::integer) {
        return std::make_unique<formula_literal_node>(line_nr, column_nr, datum{static_cast<long long>(*it++)});

    } else if (*it == token::real) {
        return std::make_unique<formula_literal_node>(line_nr, column_nr, datum{static_cast<double>(*it++)});

    } else if (*it == token::dstr) {
        return std::make_unique<formula_literal_node>(line_nr, column_nr, datum{static_cast<std::string>(*it++)});

    } else if (*it == token::id and *it == "true") {
        ++it;
        return std::make_unique<formula_literal_node>(line_nr, column_nr, datum{true});

    } else if (*it == token::id and *it == "false") {
        ++it;
        return std::make_unique<formula_literal_node>(line_nr, column_nr, datum{false});

    } else if (*it == token::id and *it == "null") {
        ++it;
        return std::make_unique<formula_literal_node>(line_nr, column_nr, datum{nullptr});

    } else if (*it == token::id and *it == "undefined") {
        ++it;
        return std::make_unique<formula_literal_node>(line_nr, column_nr, datum{});

    } else if (*it == token::id) {
        return std::make_unique<formula_name_node>(line_nr, column_nr, static_cast<std::string>(*it++));

    } else if (*it == '(') {
        ++it;
        auto subformula = parse_formula(it, last);

        if (*it == ')') {
            ++it;
        } else {
            throw parse_error(std::format("{}:{}: Expected ')' token for function call got {}.", line_nr, column_nr, *it));
        }

        return subformula;

    } else if (*it == '[') {
        ++it;

        formula_node::formula_vector values;

        // ',' is between each formula, but a ']' may follow a ',' directly.
        while (*it != ']') {
            values.push_back(parse_formula(it, last));

            if (*it == ',') {
                ++it;
            } else if (*it == ']') {
                ++it;
                break;
            } else {
                throw parse_error(
                    std::format("{}:{}: Expected ']' or ',' after a vector sub-formula. got {}", line_nr, column_nr, *it));
            }
        }

        return std::make_unique<formula_vector_literal_node>(line_nr, column_nr, std::move(values));

    } else if (*it == '{') {
        ++it;

        formula_node::formula_vector keys;
        formula_node::formula_vector values;

        // ',' is between each formula, but a ']' may follow a ',' directly.
        while (*it != '}') {
            keys.push_back(parse_formula(it, last));

            if (*it == ':') {
                ++it;
            } else {
                throw parse_error(std::format("{}:{}: Expected ':' after a map key. got {}", line_nr, column_nr, *it));
            }

            values.push_back(parse_formula(it, last));

            if (*it == ',') {
                ++it;
            } else if (*it == '}') {
                ++it;
                break;
            } else {
                throw parse_error(
                    std::format("{}:{}: Expected ']' or ',' after a vector sub-formula. got {}.", line_nr, column_nr, *it));
            }
        }

        return std::make_unique<formula_map_literal_node>(line_nr, column_nr, std::move(keys), std::move(values));

    } else if (*it == token::other) {
        hilet unary_op = *it++;
        hilet[precedence, left_to_right] = operator_precedence(unary_op, false);
        auto sub_sub_expression = parse_primary_formula(it, last);
        auto sub_expression = parse_formula_1(it, last, std::move(sub_sub_expression), precedence);
        return parse_operation_formula({}, unary_op, std::move(sub_expression));

    } else {
        throw parse_error(std::format("{}:{}: Unexpected token in primary formula {}.", line_nr, column_nr, *it));
    }
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] std::unique_ptr<formula_node> parse_index_formula(It& it, ItEnd last)
{
    hi_assert(it != last);

    auto rhs = parse_formula(it, last);

    if (*it == ']') {
        ++it;
    } else {
        throw parse_error(std::format("{}:{}: Expected ']' token at end of indexing operator got {}.", it->line_nr, it->column_nr, *it));
    }
    return rhs;
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] std::unique_ptr<formula_node> parse_ternary_argument_formula(It& it, ItEnd last)
{
    hi_assert(it != last);

    auto rhs_true = parse_formula(it, last);

    if (*it == ':') {
        ++it;
    } else {
        throw parse_error(std::format("{}:{}: Expected ':' token in ternary formula {}.", it->line_nr, it->column_nr, *it));
    }

    auto rhs_false = parse_formula(it, last);

    return std::make_unique<formula_arguments>(it->line_nr, it->column_nr, std::move(rhs_true), std::move(rhs_false));
}

/** Parse the rhs of an index operator, including the closing bracket.
 */
template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] std::unique_ptr<formula_node> parse_call_argument_formula(It& it, ItEnd last)
{
    hi_assert(it != last);

    formula_node::formula_vector args;

    if (*it == ')') {
        ++it;

    } else
        while (true) {
            args.push_back(parse_formula(it, last));

            if (*it == ',') {
                ++it;
                continue;

            } else if (*it == ')') {
                ++it;
                break;

            } else {
                throw parse_error(
                    std::format("{}:{}: Expected ',' or ')' After a function argument {}.", it->line_nr, it->column_nr, *it));
            }
        }

    return std::make_unique<formula_arguments>(it->line_nr, it->column_nr, std::move(args));
}

template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] bool parse_formula_is_at_end(It& it, ItEnd last)
{
    if (it == last) {
        return true;
    }

    if (*it != token::other) {
        throw parse_error(std::format("{}:{}: Expecting an operator token got {}.", it->line_nr, it->column_nr, *it));
    }

    return *it == ')' or *it == '}' or *it == ']' or *it == ':' or *it == ',';
}

/** Parse an formula.
 * https://en.wikipedia.org/wiki/Operator-precedence_parser
 * Parses an formula until EOF, ')', '}', ']', ':', ','
 */
template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] std::unique_ptr<formula_node> parse_formula_1(It& it, ItEnd last, std::unique_ptr<formula_node> lhs, uint8_t min_precedence)
{
    auto lookahead = token{};
    auto lookahead_precedence = uint8_t{};
    auto lookahead_left_to_right = false;

    std::tie(lookahead_precedence, lookahead_left_to_right) = operator_precedence(lookahead = *it, true);
    if (parse_formula_is_at_end(it, last)) {
        return lhs;
    }

    while (lookahead_precedence >= min_precedence) {
        hilet op = lookahead;
        hilet op_precedence = lookahead_precedence;
        ++it;

        std::unique_ptr<formula_node> rhs;
        if (op == '[') {
            rhs = parse_index_formula(it, last);
        } else if (op == '(') {
            rhs = parse_call_argument_formula(it, last);
        } else if (op == '?') {
            rhs = parse_ternary_argument_formula(it, last);
        } else {
            rhs = parse_primary_formula(it, last);
        }

        std::tie(lookahead_precedence, lookahead_left_to_right) = operator_precedence(lookahead = *it, true);
        if (parse_formula_is_at_end(it, last)) {
            return parse_operation_formula(std::move(lhs), op, std::move(rhs));
        }

        while ((lookahead_left_to_right == true && lookahead_precedence > op_precedence) ||
               (lookahead_left_to_right == false && lookahead_precedence == op_precedence)) {
            rhs = parse_formula_1(it, last, std::move(rhs), lookahead_precedence);

            std::tie(lookahead_precedence, lookahead_left_to_right) = operator_precedence(lookahead = *it, true);
            if (parse_formula_is_at_end(it, last)) {
                return parse_operation_formula(std::move(lhs), op, std::move(rhs));
            }
        }
        lhs = parse_operation_formula(std::move(lhs), op, std::move(rhs));
    }

    return lhs;
}

/** Parse an formula.
 * Parses an formula until EOF, ')', ',', '}'
 */
template<std::input_iterator It, std::sentinel_for<It> ItEnd>
[[nodiscard]] std::unique_ptr<formula_node> parse_formula(It& it, ItEnd last)
{
    auto sub_expression = parse_primary_formula(it, last);
    return parse_formula_1(it, last, std::move(sub_expression), 0);
}

/** Parse an formula.
 * Parses an formula until EOF, ')', ',', '}'
 */
[[nodiscard]] std::unique_ptr<formula_node> parse_formula_without_post_processing(std::string_view text)
{
    auto token_it = lexer<lexer_config::c_style()>.parse(text.begin(), text.end());
    return parse_formula(token_it, std::default_sentinel);
}

/** Parse an formula.
 * Parses an formula until EOF, ')', ',', '}'
 */
[[nodiscard]] std::unique_ptr<formula_node> parse_formula(std::string_view text, formula_post_process_context post_process_context = {})
{
    auto e = parse_formula_without_post_processing(text);
    e->post_process(post_process_context);
    return e;
}

/** Find the end of an formula.
 * This function will track nested brackets and strings, until the terminating_character is found.
 * @param first Iterator to the first character of the formula.
 * @param last Iterator to beyond the last character of the text.
 * @param terminating_string The string to find, which is not part of the formula.
 * @return Iterator to the terminating character if found, or last.
 */
[[nodiscard]] constexpr std::string_view::const_iterator find_end_of_formula(
    std::string_view::const_iterator first,
    std::string_view::const_iterator last,
    std::string_view terminating_string) noexcept
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
            case '"':
                in_string = '"';
                break;
            case '\'':
                in_string = '\'';
                break;
            case '{':
                bracket_stack += '}';
                break;
            case '[':
                bracket_stack += ']';
                break;
            case '(':
                bracket_stack += ')';
                break;
            case '\\':
                in_escape = true;
                break; // It is possible to escape any character, including the terminating_character.
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

}} // namespace hi::inline v1
