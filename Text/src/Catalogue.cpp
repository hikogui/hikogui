
namespace TTauri::Text {

[[nodiscard]] static parse_result_t<std::tuple<std::string,int,std::string>> parseLine(token_iterator token)
{
    std::string name;

    if ((*token == tokenizer_name_t::Name)) {
        name = static_cast<std::string>(*token++);
    } else {
        TTAURI_THROW(parse_error("Expecting a name at start of each line").set_location(token->location));
    }

    int index = std::numeric_limits<int>::max();
    if ((*token == tokenizer_name_t::Operator) && (*token == "[")) {
        token++;

        if ((*token == tokenizer_name_t::IntegerLiteral)) {
            name = static_cast<std::string>(*token++);
        } else {
            TTAURI_THROW(parse_error("Expecting an integer literal as in index for {}", name).set_location(token->location));
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
            return {name, index, value};
        }
    }
}

[[nodiscard]] static parse_result_t<> parseEntry(token_iterator token)
{
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
}

[[nodiscard]] Catalogue parseCatalogue(std::string_view text)
{
    auto tokens = parseTokens(text);

    ttauri_assume(tokens.back() == tokenizer_name_t::End);

    Catalogue r;
    auto token = tokens.begin();
    while (*token != tokenizer_name_t::End) {
        if (auto result = parseEntry(content, token)) {
            catalogue.msgstr[result.value.msgstr] = result.value.translation;
            token = resut.next_token;
        }
    }

    return r;
}

[[nodiscard]] Catalogue parseCatalogue(URL const &url)
{
    auto text = FileView(url);
    return parseCatalogue(text.string_view());
}


}
