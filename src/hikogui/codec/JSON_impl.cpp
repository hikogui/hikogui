// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "JSON.hpp"
#include "../file/file_view.hpp"

namespace hi::inline v1 {

struct parse_context_t {
    std::string_view::const_iterator text_begin;
};

[[nodiscard]] static parse_result<datum> parseValue(parse_context_t &context, token_iterator token);

[[nodiscard]] static parse_result<datum> parseArray(parse_context_t &context, token_iterator token)
{
    auto v = datum::make_vector();

    // Required '['
    if ((*token == tokenizer_name_t::Operator) && (*token == "[")) {
        token++;
    } else {
        return {};
    }

    bool commaAfterValue = true;
    while (true) {
        // A ']' is required at end of configuration-items.
        if ((*token == tokenizer_name_t::Operator) && (*token == "]")) {
            token++;
            break;

            // Required a value.
        } else if (auto result = parseValue(context, token)) {
            if (!commaAfterValue) {
                throw parse_error(std::format("{}: Missing expected ','", token->location));
            }

            v.push_back(*result);
            token = result.next_token;

            if ((*token == tokenizer_name_t::Operator) && (*token == ",")) {
                token++;
                commaAfterValue = true;
            } else {
                commaAfterValue = false;
            }

        } else {
            throw parse_error(std::format("{}: Expecting a value as the next item in an array.", token->location));
        }
    }

    return {std::move(v), token};
}

[[nodiscard]] static parse_result<datum> parseObject(parse_context_t &context, token_iterator token)
{
    auto object = datum::make_map();

    // Required '{'
    if ((*token == tokenizer_name_t::Operator) && (*token == "{")) {
        token++;
    } else {
        return {};
    }

    bool commaAfterValue = true;
    while (true) {
        // A '}' is required at end of configuration-items.
        if ((*token == tokenizer_name_t::Operator) && (*token == "}")) {
            token++;
            break;

            // Required a string name.
        } else if (*token == tokenizer_name_t::StringLiteral) {
            if (!commaAfterValue) {
                throw parse_error(std::format("{}: Missing expected ','", token->location));
            }

            auto name = static_cast<std::string>(*token++);

            if ((*token == tokenizer_name_t::Operator) && (*token == ":")) {
                token++;
            } else {
                throw parse_error(std::format("{}: Missing expected ':'", token->location));
            }

            if (auto result = parseValue(context, token)) {
                object[name] = *result;
                token = result.next_token;

            } else {
                throw parse_error(std::format("{}: Missing JSON value", token->location));
            }

            if ((*token == tokenizer_name_t::Operator) && (*token == ",")) {
                token++;
                commaAfterValue = true;
            } else {
                commaAfterValue = false;
            }

        } else {
            throw parse_error(std::format("{}: Unexpected token {}, expected a key or close-brace.", token->location, *token));
        }
    }

    return {std::move(object), token};
}

[[nodiscard]] static parse_result<datum> parseValue(parse_context_t &context, token_iterator token)
{
    switch (token->name) {
    case tokenizer_name_t::StringLiteral: {
        auto value = datum{static_cast<std::string>(*token++)};
        return {std::move(value), token};
    } break;
    case tokenizer_name_t::IntegerLiteral: {
        auto value = datum{static_cast<long long>(*token++)};
        return {std::move(value), token};
    } break;
    case tokenizer_name_t::FloatLiteral: {
        auto value = datum{static_cast<double>(*token++)};
        return {std::move(value), token};
    } break;
    case tokenizer_name_t::Name: {
        hilet name = static_cast<std::string>(*token++);
        if (name == "true") {
            return {datum{true}, token};
        } else if (name == "false") {
            return {datum{false}, token};
        } else if (name == "null") {
            return {datum{nullptr}, token};
        } else {
            throw parse_error(std::format("{}: Unexpected name '{}'", token->location, name));
        }
    } break;
    default:
        if (auto result1 = parseObject(context, token)) {
            return result1;
        } else if (auto result2 = parseArray(context, token)) {
            return result2;
        } else {
            throw parse_error(std::format("{}: Unexpected token '{}'", token->location, token->name));
        }
    }
}

[[nodiscard]] datum parse_JSON(std::string_view text)
{
    token_vector tokens = parseTokens(text);

    datum root;

    hi_assert(tokens.back() == tokenizer_name_t::End);
    parse_context_t context;
    context.text_begin = text.begin();

    auto token = tokens.begin();

    if (auto result = parseValue(context, token)) {
        root = std::move(*result);
        token = result.next_token;

    } else {
        throw parse_error(std::format("{}: Missing JSON object", token->location));
    }

    if (*token != tokenizer_name_t::End) {
        throw parse_error(std::format("{}: Unexpected text after JSON root object", token->location));
    }

    return root;
}


static void format_JSON_impl(datum const &value, std::string &result, hi::indent indent = {})
{
    if (holds_alternative<nullptr_t>(value)) {
        result += "null";
    } else if (hilet *b = get_if<bool>(value)) {
        result += *b ? "true" : "false";
    } else if (hilet *i = get_if<long long>(value)) {
        result += hi::to_string(*i);
    } else if (hilet *f = get_if<double>(value)) {
        result += hi::to_string(*f);
    } else if (hilet *s = get_if<std::string>(value)) {
        result += '"';
        for (hilet c : *s) {
            switch (c) {
            case '\n':
                result += '\\';
                result += 'n';
                break;
            case '\r':
                result += '\\';
                result += 'r';
                break;
            case '\t':
                result += '\\';
                result += 't';
                break;
            case '\f':
                result += '\\';
                result += 'f';
                break;
            case '"':
                result += '\\';
                result += '"';
                break;
            default: result += c;
            }
        }
        result += '"';

    } else if (hilet *v = get_if<datum::vector_type>(value)) {
        result += indent;
        result += '[';
        result += '\n';

        for (auto it = v->begin(); it != v->end(); it++) {
            if (it != v->begin()) {
                result += ',';
                result += '\n';
            }
            result += indent + 1;

            format_JSON_impl(*it, result, indent + 1);
        }

        result += '\n';
        result += indent;
        result += ']';
    } else if (hilet *m = get_if<datum::map_type>(value)) {
        result += indent;
        result += '{';
        result += '\n';

        for (auto it = m->begin(); it != m->end(); it++) {
            if (it != m->begin()) {
                result += ',';
                result += '\n';
            }
            result += indent + 1;

            format_JSON_impl(it->first, result, indent + 1);
            result += ':';
            result += ' ';
            format_JSON_impl(it->second, result, indent + 1);
        }

        result += '\n';
        result += indent;
        result += '}';
    } else {
        hi_no_default();
    }
}

[[nodiscard]] std::string format_JSON(datum const &root)
{
    auto r = std::string{};
    format_JSON_impl(root, r);
    r += '\n';
    return r;
}
} // namespace hi::inline v1
