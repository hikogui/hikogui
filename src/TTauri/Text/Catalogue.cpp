
#pragma once

#include "Catalogue.hpp"
#include "TTauri/Foundation/ResourceView.hpp"
#include "TTauri/Foundation/tokenizer.hpp"
#include "language.hpp"

namespace tt {

///** Return the plarity index.
//*/
//ssize_t plurality(long long n) const noexcept {
//    // To protect against overflow make the number smaller,
//    // But preserve trailing digits since language rules check for these.
//    n = (n > 1000000) ? (n % 1000000) : n;
//
//    auto context = expression_evaluation_context{};
//    context.set_local("n", n);
//
//    if (plural_expression) {
//        ttlet result = plural_expression->evaluate(context);
//        if (result.is_bool()) {
//            return static_cast<bool>(result) ? 1 : 0;
//        } else if (result.is_integer()) {
//            return static_cast<ssize_t>(result);
//        } else {
//            LOG_ERROR("Language {}: plurality expression with value {} results in non-bool or non-integer type but {}",
//                name, n, result.type_name()
//            );
//            // Plural expression failure, use english rules.
//            return (n == 1) ? 0 : 1;
//        }
//
//    } else {
//        // No plural expression available, use english rules.
//        return (n == 1) ? 0 : 1;
//    }
//}

TranslationCatalogue::TranslationCatalogue() noexcept
{
    language_list_cbid = language_list.add_callback([this](ttlet &new_language_list) {
        set_prefered_languages(new_language_list);
    });

    set_prefered_languages(*language_list);
}

TranslationCatalogue::~TranslationCatalogue()
{
    language_list.remove_callback(language_list_cbid);
}

void TranslationCatalogue::set_prefered_languages(std::vector<std::string> const &new_language_list) noexcept
{
    prefered_language_ptrs.clear();
    for (ttlet &new_language: new_language_list) {
        for (ttlet &language: languages) {
            if (language->name == new_language) {
                prefered_language_ptrs.push_back(language.get());
            }
        }
    }
}

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

[[nodiscard]] TranslationCatalogue parseCatalogue(std::string_view text)
{
    auto tokens = parseTokens(text);

    tt_assume(tokens.back() == tokenizer_name_t::End);

    TranslationCatalogue r;
    auto token = tokens.begin();
    while (*token != tokenizer_name_t::End) {
        if (auto result = parseEntry(token)) {
            r.add_translation(result.value->original, result.value->translation);
            token = result.next_token;
        }
    }

    return r;
}

[[nodiscard]] TranslationCatalogue parseCatalogue(URL const &url)
{
    ttlet text = url.loadView();
    return parseCatalogue(text->string_view());
}


}
