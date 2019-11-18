// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/tokenizer.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/datum.hpp"
#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Foundation/JSONParser.hpp"
#include <vector>
#include <optional>

namespace Messari {

using namespace TTauri;


static datum parseValue(parse_context_t &context, token_iterator token);

static datum parseArray(parse_context_t &context, token_iterator token)
{
    auto array = datum{datum_vector{}};

    // Required '['
    if ((*token == tokenizer_name_t::Literal) && (*token == "[")) {
        token++; 
    } else {
        return {};
    }

    bool commaAfterValue = true;
    while (true) {
        // A ']' is required at end of configuration-items.
        if ((*token == tokenizer_name_t::Literal) && (*token == "]")) {
            token++;
            break;

        // Required a value.
        } else if (auto result = parseValue(context, token)) {
            if (!commaAfterValue) {
                let [line, column] = context.line_and_column(token);
                TTAURI_THROW(parse_error("Missing expected ','")
                    .set<"line"_tag>(line)
                    .set<"column"_tag>(column)
                );
            }

            array.push_back(*result.value);

            if ((*token == tokenizer_name_t::Literal) && (*token == ",")) {
                token++;
                commaAfterValue = true;
            } else {
                commaAfterValue = false;
            }

        } else {
            let [line, column] = context.line_and_column(token);
            TTAURI_THROW(parse_error("Expecting a value as the next item in an array.")
                .set<"line"_tag>(line)
                .set<"column"_tag>(column)
            );
        }
    }

    return {std::move(array), token};
}

static datum parseObject(parse_context_t &context, token_iterator token)
{
    auto object = datum{datum_map{}};

    // Required '{'
    if ((*token == tokenizer_name_t::Literal) && (*token == "{")) {
        token++; 
    } else {
        return {};
    }

    bool commaAfterValue = true;
    while (true) {
        // A '}' is required at end of configuration-items.
        if ((*token == tokenizer_name_t::Literal) && (*token == "}")) {
            token++;
            break;

        // Required a string name.
        } else if (*token == tokenizer_name_t::StringLiteral) {
            if (!commaAfterValue) {
                let [line, column] = context.line_and_column(token);
                TTAURI_THROW(parse_error("Missing expected ','")
                    .set<"line"_tag>(line)
                    .set<"column"_tag>(column)
                );
            }

            auto name = static_cast<std::string>(*token++);

            if ((*token == tokenizer_name_t::Literal) && (*token == ":")) {
                token++;
            } else {
                let [line, column] = context.line_and_column(token);
                TTAURI_THROW(parse_error("Missing expected ':'")
                    .set<"line"_tag>(line)
                    .set<"column"_tag>(column)
                );
            }

            if (auto result = parseValue(context, token)) {
                object[name] = *result.value;

            } else {
                let [line, column] = context.line_and_column(token);
                TTAURI_THROW(parse_error("Missing JSON value")
                    .set<"line"_tag>(line)
                    .set<"column"_tag>(column)
                );
            }

            if ((*token == tokenizer_name_t::Literal) && (*token == ",")) {
                token++;
                commaAfterValue = true;
            } else {
                commaAfterValue = false;
            }

        } else {
            let [line, column] = context.line_and_column(token);
            TTAURI_THROW(parse_error("Expecting a key-string as the next item in an object.")
                .set<"line"_tag>(line)
                .set<"column"_tag>(column)
            );
        }
    }

    return {std::move(object), token};
}

static datum parseValue(parse_context_t &context, token_iterator token)
{
    switch (token->name) {
    case tokenizer_name_t::StringLiteral:
        return datum{static_cast<std::string>(*token++);
    case tokenizer_name_t::IntegerLiteral:
        return datum{static_cast<long long>(*token++);
    case tokenizer_name_t::FloatLiteral:
        return datum{static_cast<double>(*token++);
    case tokenizer_name_t::Name:
        let name = static_cast<std::string>(*token);
        if (name == "true") {
            return datum{true};
        } else if (name == "false") {
            return datum{false};
        } else if (name == "null") {
            return datum{datum::null};
        } else {
            let [line, column] = context.line_and_column(token);
            TTAURI_THROW(parse_error("Unexpected name '{}'", name)
                .set<"line"_tag>(line)
                .set<"column"_tag>(column)
            );
        }
    default:
        if (auto result = parseObject(context, token)) {
            return std::move(*result.value);
        } else if (auto result = parseArray(context, token)) {
            return std::move(*result.value);
        } else {
            let [line, column] = context.line_and_column(token);
            TTAURI_THROW(parse_error("Unexpected token", name)
                .set<"line"_tag>(line)
                .set<"column"_tag>(column)
            );
        }
    }
}

datum parseJSON(std::string_view text)
{
    token_vector tokens = parseTokens(text);

    datum root;

    axiom_assert(tokens.back() == tokenizer_name_t::End);
    parse_context_t context;
    context.text_begin = text.begin();

    auto token = tokens.begin();

    if (auto result = parseObject(context, token)) {
        root = std::move(*result.value);

    } else {
        let [line, column] = context.line_and_column(token);
        TTAURI_THROW(parse_error("Missing JSON object")
            .set<"line"_tag>(line)
            .set<"column"_tag>(column)
        );
    }

    if (*token != tokenizer_name_t::End) {
        let [line, column] = context.line_and_column(token);
        TTAURI_THROW(parse_error("Unexpected text after JSON root object")
            .set<"line"_tag>(line)
            .set<"column"_tag>(column)
        );
    }

    return root;
}

datum parseJSON(TTauri::URL const &file)
{
    auto view = TTauri::FileView(file);
    return parseJSON(view.string_view());
}

static void dumpJSON_impl(datum const &value, std::string &result, int indent=0)
{
    bool first_item = true;

    switch (value.type()) {
    case datum_type_t::Null:
        result += "null";
        break;

    case datm_type_t::Boolean:
        result += value ? "true" : "false";
        break;

    case datm_type_t::Integer:
        result += fmt::format("{}", static_cast<long long>(value)
        break;

    case datm_type_t::Float:
        result += fmt::format("{}", static_cast<double>(value)
        break;

    case datm_type_t::String:
    case datm_type_t::URL:
        result += '"';
        for (let c: static_cast<std::string>(value)) {
            switch (c) {
            case '\n': result += '\\'; result += 'n'; break;
            case '\r': result += '\\'; result += 'r'; break;
            case '\t': result += '\\'; result += 't'; break;
            case '\f': result += '\\'; result += 'f'; break;
            case '"': result += '\\'; result += '"'; break;
            default: result += c;
            }
        }
        result += '"';
        break;

    case datum_type_t::wsRGBA:
        auto color_value = static_cast<wsRGBA>(value);
        result += '"';
        result += to_string(color_value);
        result += '"';
        break;

    case datum_type_t::Vector:
        result.append(' ', indent);
        result += '[';

        for (auto i = value.vector_begin(); i != value.vector_end(); i++, first_item = true) {
            if (!first_item) {
                result += ',';
                result += '\n';
            }
            result.append(' ', indent + 1);

            dumpJSON_impl(*i, result, indent + 1);
        }

        result += '\n';
        result.append(' ', indent);
        result += ']';
        break;

    case datum_type_t::Map:
        result.append(' ', indent);
        result += '{';

        for (auto i = value.map_begin(); i != value.map_end(); i++, first_item = true) {
            if (!first_item) {
                result += ',';
                result += '\n';
            }
            result.append(' ', indent + 1);

            dumpJSON_impl(i->first, result, indent + 1);
            result += ':';
            result += ' ';
            dumpJSON_impl(i->second, result, indent + 1);
        }

        result += '\n';
        result.append(' ', indent);
        result += '}';
        break;

    default:
        no_default;
    }
}

std::string dumpJSON(datum const &root)
{
    auto r = std::string{};
    dumpJSON_impl(root, r);
    return r;
}


}
