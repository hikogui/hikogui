// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "po_parser.hpp"
#include "language.hpp"
#include "translation.hpp"
#include "../tokenizer.hpp"

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
//            tt_log_error("Language {}: plurality expression with value {} results in non-bool or non-integer type but {}",
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

[[nodiscard]] static parse_result<std::tuple<std::string,int,std::u8string>> parseLine(token_iterator token)
{
    std::string name;
    if ((*token == tokenizer_name_t::Name)) {
        name = static_cast<std::string>(*token++);
    } else {
        throw parse_error("{}: Expecting a name at start of each line", token->location);
    }

    int index = 0;
    if ((*token == tokenizer_name_t::Operator) && (*token == "[")) {
        token++;

        if ((*token == tokenizer_name_t::IntegerLiteral)) {
            index = static_cast<int>(*token++);
        } else {
            throw parse_error("{}: Expecting an integer literal as an index for {}", token->location, name);
        }


        if ((*token == tokenizer_name_t::Operator) && (*token == "]")) {
            token++;
        } else {
            throw parse_error("{}: The index on {} must terminate with a bracket ']'", token->location, name);
        }
    }

    std::u8string value;
    if ((*token == tokenizer_name_t::StringLiteral)) {
        value = static_cast<std::u8string>(*token++);
    } else {
        throw parse_error("{}: Expecting a value at end of each line", token->location);
    }

    while (true) {
        if ((*token == tokenizer_name_t::StringLiteral)) {
            value += static_cast<std::u8string>(*token++);
        } else {
            return {std::tuple{name, index, value}, token};
        }
    }
}

[[nodiscard]] static parse_result<po_translation> parse_po_translation(token_iterator token)
{
    po_translation r;

    while (true) {
        if (std::ssize(r.msgstr) == 0) {
            if (auto result = parseLine(token)) {
                token = result.next_token;

                ttlet [name, index, value] = *result;
                if (name == "msgctxt") {
                    r.msgctxt = value;

                } else if (name == "msgid") {
                    r.msgid = value;

                } else if (name == "msgid_plural") {
                    r.msgid_plural = value;

                } else if (name == "msgstr") {
                    while (std::ssize(r.msgstr) <= index) {
                        r.msgstr.push_back({});
                    }
                    r.msgstr[index] = value;

                } else {
                    throw parse_error("{}: Unexpected line {}", token->location, name);
                }

            } else {
                return {};
            }

        } else if ((*token == tokenizer_name_t::Name) && (*token == "msgstr")) {
            if (auto result = parseLine(token)) {
                token = result.next_token;
                ttlet [name, index, value] = *result;

                while (std::ssize(r.msgstr) <= index) {
                    r.msgstr.push_back({});
                }
                r.msgstr[index] = value;

            } else {
                return {};
            }

        } else {
            return {r, token};
        }
    }
}

static void parse_po_header(po_translations &r, std::string const &header)
{
    for (ttlet &line : split(header, '\n')) {
        if (std::ssize(line) == 0) {
            // Skip empty header lines.
            continue;
        }

        auto split_line = split(line, ':');
        if (std::ssize(split_line) < 2) {
            throw parse_error("Unknown header '{}'", line);
        }

        ttlet name = split_line.front();
        split_line.erase(split_line.begin());
        ttlet value = join(split_line, ":");

        if (name == "Language") {
            r.language = language_tag{value};
        } else if (name == "Plural-Forms") {
            ttlet plural_split = split(value, ';');
        }
    }
}

[[nodiscard]] static po_translations parse_po(std::string_view text)
{
    po_translations r;

    auto tokens = parseTokens(text);
    tt_axiom(tokens.back() == tokenizer_name_t::End);

    auto token = tokens.begin();
    while (*token != tokenizer_name_t::End) {
        if (auto result = parse_po_translation(token)) {
            token = result.next_token;

            if (std::ssize(result.value.msgid) != 0) {
                r.translations.push_back(result.value);

            } else if (std::ssize(result.value.msgstr) == 1) {
                parse_po_header(r, tt::to_string(result.value.msgstr.front()));

            } else {
                throw parse_error("Unknown .po header");
            }
        }
    }

    return r;
}

[[nodiscard]] po_translations parse_po(URL const &url)
{
    ttlet text = url.loadView();
    return parse_po(text->string_view());
}

}

