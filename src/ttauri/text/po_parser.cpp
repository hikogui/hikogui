
#include "po_parser.hpp"
#include "language.hpp"
#include "translation.hpp"
#include "ttauri/foundation/ResourceView.hpp"
#include "ttauri/foundation/tokenizer.hpp"

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

[[nodiscard]] static parse_result<std::tuple<std::string,int,std::string>> parseLine(token_iterator token)
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

[[nodiscard]] static parse_result<po_translation> parse_po_translation(token_iterator token)
{
    po_translation r;

    while (true) {
        if (ssize(r.msgstr) == 0) {
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
                    while (ssize(r.msgstr) <= index) {
                        r.msgstr.push_back({});
                    }
                    r.msgstr[index] = value;

                } else {
                    TTAURI_THROW(parse_error("Unexpected line {}", name).set_location(token->location));
                }

            } else {
                return {};
            }

        } else if ((*token == tokenizer_name_t::Name) && (*token == "msgstr")) {
            if (auto result = parseLine(token)) {
                token = result.next_token;
                ttlet [name, index, value] = *result;

                while (ssize(r.msgstr) <= index) {
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
    for (ttlet &line : split(header, "\n")) {
        if (ssize(line) == 0) {
            // Skip empty header lines.
            continue;
        }

        auto split_line = split(line, ":");
        if (ssize(split_line) < 2) {
            TTAURI_THROW(parse_error("Unknown header '{}'", line));
        }

        ttlet name = split_line.front();
        split_line.erase(split_line.begin());
        ttlet value = join(split_line, ":");

        if (name == "Language") {
            r.language = value;
        } else if (name == "Plural-Forms") {
            ttlet plural_split = split(value, ";");
        }
    }
}

[[nodiscard]] static po_translations parse_po(std::string_view text)
{
    po_translations r;

    auto tokens = parseTokens(text);
    tt_assume(tokens.back() == tokenizer_name_t::End);

    auto token = tokens.begin();
    while (*token != tokenizer_name_t::End) {
        if (auto result = parse_po_translation(token)) {
            token = result.next_token;

            if (ssize(result.value.msgid) != 0) {
                r.translations.push_back(result.value);

            } else if (ssize(result.value.msgstr) == 1) {
                parse_po_header(r, result.value.msgstr.front());

            } else {
                TTAURI_THROW(parse_error("Unknown .po header"));
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

