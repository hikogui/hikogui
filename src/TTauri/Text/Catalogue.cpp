
#pragma once

#include "Catalogue.hpp"
#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/tokenizer.hpp"

namespace tt {

[[nodiscard]] static parse_result_t<std::tuple<std::string,int,std::string>> parseLine(token_iterator token)
{
    std::string name;
    if ((*token == tokenizer_name_t::Name)) {
        name = static_cast<std::string>(*token++);
    } else {
        TTAURI_THROW(parse_error("Expecting a name at start of each line").set_location(token->location));
    }

    int index = 0;
    if ((*token == tokenizer_name_t::Operator) && (*token == "[")) {
        token++;

        if ((*token == tokenizer_name_t::IntegerLiteral)) {
            index = static_cast<int>(*token++);
        } else {
            TTAURI_THROW(parse_error("Expecting an integer literal as an index for {}", name).set_location(token->location));
        }


        if ((*token == tokenizer_name_t::Operator) && (*token == "]")) {
            token++;
        } else {
            TTAURI_THROW(parse_error("The index on {} must terminate with a bracket ']'", name).set_location(token->location));
        }
    }

    std::string value;
    if ((*token == tokenizer_name_t::StringLiteral)) {
        value = static_cast<std::string>(*token++);
    } else {
        TTAURI_THROW(parse_error("Expecting a value at end of each line").set_location(token->location));
    }

    while (true) {
        if ((*token == tokenizer_name_t::StringLiteral)) {
            value += static_cast<std::string>(*token++);
        } else {
            return {std::tuple{name, index, value}, token};
        }
    }
}

struct translation_t {
    std::string original;
    std::vector<std::string> translation;
};

[[nodiscard]] static parse_result_t<translation_t> parseEntry(token_iterator token)
{
    translation_t r;

    if (auto result = parseLine(token)) {
        token = result.next_token;
    }

    while (true) {
        if ((*token != tokenizer_name_t::Name) || (*token != "msgid")) {
            return {};
        }

        if (auto result = parseLine(token)) {
            token = result.next_token;
        }
    }

    return {r, token};
}

[[nodiscard]] Catalogue parseCatalogue(std::string_view text)
{
    auto tokens = parseTokens(text);

    tt_assume(tokens.back() == tokenizer_name_t::End);

    Catalogue r;
    auto token = tokens.begin();
    while (*token != tokenizer_name_t::End) {
        if (auto result = parseEntry(token)) {
            r.add_translation(result.value->original, result.value->translation);
            token = result.next_token;
        }
    }

    return r;
}

[[nodiscard]] Catalogue parseCatalogue(URL const &url)
{
    ttlet text = url.loadView();
    return parseCatalogue(text->string_view());
}


}
